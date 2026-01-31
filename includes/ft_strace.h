#ifndef FT_STRACE
# define FT_STRACE


#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#include <stdint.h>     // uint32_t
#include <sys/user.h>   // struct user_regs_struct

typedef struct syscall_s {
    const char *name;
    uint32_t argc;
} syscall_t;

/* syscall.c */
void print_syscall(struct user_regs_struct *regs);

#endif
