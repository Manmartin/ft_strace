#include <bits/types/siginfo_t.h>
#include <linux/elf.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ft_strace.h"

int trace_loop(pid_t child) {
    int                       status;
    struct user_regs_struct64 regs;
    struct iovec              iov;

    check((ptrace(PTRACE_SEIZE, child, NULL,
                  PTRACE_O_TRACESYSGOOD | PTRACE_O_EXITKILL) == -1),
          "PTRACE_SEIZE");
    waitpid(child, &status, 0);
    if (WIFSIGNALED(status))
        signal_exit(status);

    iov.iov_base = &regs;
    iov.iov_len  = sizeof(regs);

    bool in_syscall     = false;
    bool first_execve   = true;
    int  syscall_signal = 0;
    while (!WIFEXITED(status)) {
        check(ptrace(PTRACE_SYSCALL, child, NULL, syscall_signal),
              "PTRACE_SYSCALL");
        waitpid(child, &status, 0);
        syscall_signal = 0;

        if (WIFSIGNALED(status)) {
            signal_exit(status);
            return status | 0x80;
        }

        if (WIFSTOPPED(status) && (status >> 8) == (SIGTRAP | 0x80)) {
            check(ptrace(PTRACE_GETREGSET, child, NT_PRSTATUS, &iov),
                  "PTRACE_GETREGSET");
            if (!in_syscall) {
                print_syscall_input(&iov);
            } else {
                bool status = print_syscall_output(&iov);
                if (first_execve) {
                    if (status == SYSCALL_ERROR)
                        return EXIT_FAILURE;
                    if (iov.iov_len == sizeof(struct user_regs_struct32))
                        fprintf(stderr,
                                "[ Process PID=%i runs in 32 bit mode. ]\n",
                                child);
                    first_execve = false;
                }
            }
            in_syscall = !in_syscall;
        } else if (WIFSTOPPED(status)) {
            siginfo_t signal;

            syscall_signal = WSTOPSIG(status);
            ptrace(PTRACE_GETSIGINFO, child, NULL, &signal);
            print_signal(signal);
        }
    }
    if (in_syscall) {
        fprintf(stderr, " = ?\n");
    }
    fprintf(stderr, "+++ exited with %i +++\n", WEXITSTATUS(status));
    return WEXITSTATUS(status);
}
