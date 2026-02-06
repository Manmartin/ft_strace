#include <linux/elf.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ft_strace.h"

int trace_loop(pid_t child) {

  check((ptrace(PTRACE_SEIZE, child, NULL, PTRACE_O_TRACESYSGOOD) == -1),
        "PTRACE_SEIZE");

  int status;
  waitpid(child, &status, 0);

  struct user_regs_struct regs;
  struct iovec iov;
  iov.iov_base = &regs;
  iov.iov_len = sizeof(regs);

  bool in_syscall = false;
  int syscall_signal = 0;
  while (!WIFEXITED(status)) {
    check(ptrace(PTRACE_SYSCALL, child, NULL, syscall_signal),
          "PTRACE_SYSCALL");
    waitpid(child, &status, 0);
    syscall_signal = 0;

    if (WIFSIGNALED(status)) {
      fprintf(stderr, "+++ killed by %i +++\n", WTERMSIG(status));
      return status | 0x80;
    }

    if (WIFSTOPPED(status) && (status >> 8) == (SIGTRAP | 0x80)) {
      check(ptrace(PTRACE_GETREGSET, child, NT_PRSTATUS, &iov),
            "PTRACE_GETREGSET");
      if (!in_syscall && iov.iov_len == sizeof(regs)) {
        print_syscall64(&regs);
      } else if (!in_syscall) {
        print_syscall32((struct user_regs_struct32 *)&regs);
      } else if (iov.iov_len == sizeof(regs)) {
        fprintf(stderr, " = %lli\n", regs.rax);
      } else {
        fprintf(stderr, " = %i\n", ((struct user_regs_struct32 *)&regs)->eax);
      }
      in_syscall = !in_syscall;
    } else if (WIFSTOPPED(status)) {
      syscall_signal = WSTOPSIG(status);
      // TODO: print signal info
    }
  }
  if (in_syscall) {
    fprintf(stderr, " = ?\n");
  }
  fprintf(stderr, "+++ exited with %i +++\n", WEXITSTATUS(status));
  return WEXITSTATUS(status);
}
