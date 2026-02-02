#ifndef FT_STRACE
# define FT_STRACE


#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#include <stdint.h>     // uint32_t
#include <sys/user.h>   // struct user_regs_struct
#include <stdbool.h>    // bool

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

#endif
