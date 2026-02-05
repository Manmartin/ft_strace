#ifndef FT_STRACE
# define FT_STRACE


#include <signal.h>

#include <stdint.h>     // uint32_t
#include <sys/user.h>   // struct user_regs_struct
#include <stdbool.h>    // bool

#define check(function, msg)                                                   \
    if (function != 0) {                                                       \
        perror(msg);                                                           \
        exit(EXIT_FAILURE);                                                    \
    }

typedef struct args_s {
    char *program_path;
    char **args;
    char **env;
    bool timer_mode;
} args_t;

typedef struct syscall_s {
    const char *name;
    uint32_t argc;
} syscall_t;

/* args.c */
void verify_args(args_t *args, int argc, char **argv, char **env);

/* syscall.c */
void print_syscall(struct user_regs_struct *regs);

/* tracer.c */
int trace_loop(pid_t child);

#endif
