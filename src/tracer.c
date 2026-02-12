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

    check((ptrace(PTRACE_SEIZE, child, NULL, PTRACE_O_TRACESYSGOOD) == -1),
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

        if (WIFSIGNALED(status))
            signal_exit(status);

        if (WIFSTOPPED(status) && (status >> 8) == (SIGTRAP | 0x80)) {
            check(ptrace(PTRACE_GETREGSET, child, NT_PRSTATUS, &iov),
                  "PTRACE_GETREGSET");
            if (!in_syscall && iov.iov_len == sizeof(regs)) {
                print_syscall64(&regs);
            } else if (!in_syscall) {
                print_syscall32((struct user_regs_struct32 *)&regs);
            } else if (iov.iov_len == sizeof(regs)) {
                fprintf(stderr, " = %li\n", regs.rax);
                if (first_execve) {
                    first_execve = false;
                    if ((long long)regs.rax < 0) {
                        in_syscall = false;
                        return EXIT_FAILURE;
                        break;
                    }
                }
            } else {
                fprintf(stderr, " = %i\n",
                        ((struct user_regs_struct32 *)&regs)->eax);
                if (first_execve) {
                    first_execve = false;
                    if (((struct user_regs_struct32 *)&regs)->eax < 0) {
                        in_syscall = false;
                        return EXIT_FAILURE;
                    }
                }
            }
            in_syscall = !in_syscall;
        } else if (WIFSTOPPED(status)) {
            siginfo_t signal;

            syscall_signal = WSTOPSIG(status);
            ptrace(PTRACE_GETSIGINFO, child, NULL, &signal);
            print_signal(signal, WTERMSIG(status));
        }
    }
    if (in_syscall) {
        fprintf(stderr, " = ?\n");
    }
    fprintf(stderr, "+++ exited with %i +++\n", WEXITSTATUS(status));
    return WEXITSTATUS(status);
}
