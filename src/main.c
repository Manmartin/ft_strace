#include <linux/elf.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ft_strace.h"

#define check(function, msg)                                                   \
    if (function != 0) {                                                       \
        perror(msg);                                                           \
        exit(EXIT_FAILURE);                                                    \
    }

int main(int argc, char **argv, char **env) {
    (void)argc;
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }
    if (pid == 0) {
        kill(getpid(), SIGSTOP);
        execve(argv[1], &(argv[1]), env);
        perror("execve");
        exit(EXIT_FAILURE);
    }
    check(ptrace(PTRACE_SEIZE, pid, NULL, PTRACE_O_TRACESYSGOOD),
          "PTRACE_SEIZE");

    int wstatus;
    bool in_syscall = false;
    while (1) {
        check(ptrace(PTRACE_SYSCALL, pid, NULL, NULL), "PTRACE_SYSCALL");
        waitpid(pid, &wstatus, 0);

        if (WIFEXITED(wstatus)) {
            fprintf(stderr, "+++ exited with %i +++\n", WEXITSTATUS(wstatus));
            break;
        }

        if (WIFSIGNALED(wstatus)) {
            fprintf(stderr, "Exit with signal: %i\n", WTERMSIG(wstatus));
            break;
        }

        if (WIFSTOPPED(wstatus) && (wstatus >> 8) == (SIGTRAP | 0x80)) {
            if (!in_syscall) {
                in_syscall = true;
                continue;
            }
            in_syscall = false;

            struct user_regs_struct regs;
            struct iovec iov;
            iov.iov_base = &regs;
            iov.iov_len  = sizeof(regs);
            check(ptrace(PTRACE_GETREGSET, pid, NT_PRSTATUS, &iov),
                  "PTRACE_GETREGSET");
            print_syscall(&regs);
        }
    }
    return EXIT_SUCCESS;
}
