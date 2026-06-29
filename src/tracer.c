#include <errno.h>
#include <linux/elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

#include "ft_strace.h"

void set_signals_empty(void) {
    sigset_t empty;

    sigemptyset(&empty);
    check(sigprocmask(SIG_SETMASK, &empty, NULL), "SIGPROCMASK");
}

void set_signals_blocked(void) {
    sigset_t blocked;

    sigaddset(&blocked, SIGHUP);
    sigaddset(&blocked, SIGQUIT);
    sigaddset(&blocked, SIGINT);
    sigaddset(&blocked, SIGPIPE);
    sigaddset(&blocked, SIGTERM);
    check(sigprocmask(SIG_BLOCK, &blocked, NULL), "SIGPROCMASK");
}

static void wait_child(pid_t child, int *status) {
    set_signals_empty();
    if (waitpid(child, status, 0) == -1) {
        if (interrupted == 1) {
            fprintf(stderr, "strace: Process %i detached\n", child);
            exit(EXIT_FAILURE);
        } else {
            perror("WAITPID");
            exit(EXIT_FAILURE);
        }
    }
    set_signals_blocked();
}

int trace_loop(pid_t child) {
    int                       status;
    struct user_regs_struct64 regs;
    struct iovec              iov;

    check((ptrace(PTRACE_SEIZE, child, NULL,
                  PTRACE_O_TRACESYSGOOD | PTRACE_O_EXITKILL) == -1),
          "PTRACE_SEIZE");

    wait_child(child, &status);

    if (WIFSIGNALED(status))
        signal_exit(status);

    iov.iov_base = &regs;
    iov.iov_len  = sizeof(regs);

    bool in_syscall     = false;
    bool log_syscall    = true;
    bool first_execve   = true;
    int  syscall_signal = 0;
    while (!WIFEXITED(status)) {
        check(ptrace(PTRACE_SYSCALL, child, NULL, syscall_signal),
              "PTRACE_SYSCALL");
        wait_child(child, &status);
        syscall_signal = 0;

        if (WIFSIGNALED(status)) {
            signal_exit(status);
            return status | 0x80;
        }

        if (WIFSTOPPED(status) && (status >> 8) == (SIGTRAP | 0x80)) {
            check(ptrace(PTRACE_GETREGSET, child, NT_PRSTATUS, &iov),
                  "PTRACE_GETREGSET");
            if (!in_syscall) {
                if (log_syscall)
                    print_syscall_input(&iov);
            } else {
                bool status = SYSCALL_SUCCESS;
                if (log_syscall)
                    status = print_syscall_output(&iov);
                if (first_execve) {
                    if (status == SYSCALL_ERROR)
                        log_syscall = false;
                    else if (iov.iov_len == sizeof(struct user_regs_struct32))
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
    if (in_syscall && log_syscall) {
        fprintf(stderr, " = ?\n");
    }
    fprintf(stderr, "+++ exited with %i +++\n", WEXITSTATUS(status));
    return WEXITSTATUS(status);
}
