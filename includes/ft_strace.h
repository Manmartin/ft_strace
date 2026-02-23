#ifndef FT_STRACE
#define FT_STRACE

#include <bits/types/struct_iovec.h>
#include <signal.h>  // pid_t
#include <stdbool.h> // bool
#include <stdint.h>  // uint

#define SYSCALL_SUCCESS true
#define SYSCALL_ERROR false

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

typedef struct syscall_timer_s {
    uint64_t useconds;
} syscall_timer;

typedef struct timer_array_s {
    uint64_t       capacity;
    uint64_t       size;
    syscall_timer *timers;
} timer_array;

typedef struct syscall_s {
    const char *name;
    uint32_t    argc;
} syscall_t;

/* structs from sys/user.h */

struct user_regs_struct64 {
    unsigned long long int r15;
    unsigned long long int r14;
    unsigned long long int r13;
    unsigned long long int r12;
    unsigned long long int rbp;
    unsigned long long int rbx;
    unsigned long long int r11;
    unsigned long long int r10;
    unsigned long long int r9;
    unsigned long long int r8;
    unsigned long long int rax;
    unsigned long long int rcx;
    unsigned long long int rdx;
    unsigned long long int rsi;
    unsigned long long int rdi;
    unsigned long long int orig_rax;
    unsigned long long int rip;
    unsigned long long int cs;
    unsigned long long int eflags;
    unsigned long long int rsp;
    unsigned long long int ss;
    unsigned long long int fs_base;
    unsigned long long int gs_base;
    unsigned long long int ds;
    unsigned long long int es;
    unsigned long long int fs;
    unsigned long long int gs;
};

struct user_regs_struct32 {
    int ebx;
    int ecx;
    int edx;
    int esi;
    int edi;
    int ebp;
    int eax;
    int xds;
    int xes;
    int xfs;
    int xgs;
    int orig_eax;
    int eip;
    int xcs;
    int eflags;
    int esp;
    int xss;
};

/* args.c */
void verify_args(args_t *args, int argc, char **argv, char **env);

/* signals.c */
void signal_exit(int status);
void print_signal(siginfo_t signal);

/* syscall.c */
void print_syscall_input(struct iovec *iov);
bool print_syscall_output(struct iovec *iov);

/* timer.c */
int timer_loop(pid_t child);

/* tracer.c */
int trace_loop(pid_t child);

#endif
