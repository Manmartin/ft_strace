#ifndef FT_STRACE
#define FT_STRACE

#include <signal.h>  // pid_t
#include <stdbool.h> // bool
#include <stdint.h>  // uint32_t

#define check(function, msg)                                                   \
    if (function != 0) {                                                       \
        perror(msg);                                                           \
        exit(EXIT_FAILURE);                                                    \
    }

typedef struct args_s {
    char  *program_path;
    char **args;
    char **env;
    bool   timer_mode;
} args_t;

typedef struct syscall_s {
    const char *name;
    uint32_t    argc;
} syscall_t;

/* structs from sys/user.h */

struct user_regs_struct64 {
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t rbp;
    uint64_t rbx;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rax;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t orig_rax;
    uint64_t rip;
    uint64_t cs;
    uint64_t eflags;
    uint64_t rsp;
    uint64_t ss;
    uint64_t fs_base;
    uint64_t gs_base;
    uint64_t ds;
    uint64_t es;
    uint64_t fs;
    uint64_t gs;
};

struct user_regs_struct32 {
    int32_t ebx;
    int32_t ecx;
    int32_t edx;
    int32_t esi;
    int32_t edi;
    int32_t ebp;
    int32_t eax;
    int32_t xds;
    int32_t xes;
    int32_t xfs;
    int32_t xgs;
    int32_t orig_eax;
    int32_t eip;
    int32_t xcs;
    int32_t eflags;
    int32_t esp;
    int32_t xss;
};

/* args.c */
void verify_args(args_t *args, int argc, char **argv, char **env);

/* signals.c */
void signal_exit(int status);
void print_signal(siginfo_t signal);

/* syscall.c */
void print_syscall64(struct user_regs_struct64 *regs);
void print_syscall32(struct user_regs_struct32 *regs);

/* tracer.c */
int trace_loop(pid_t child);

#endif
