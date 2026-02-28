#include <bits/types/siginfo_t.h>
#include <linux/elf.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "ft_strace.h"

#define ARRAY_INITIAL_CAPACITY 32

static void init_array(timer_array *array) {
    array->capacity = ARRAY_INITIAL_CAPACITY;
    array->size     = 0;
    array->timers   = malloc(ARRAY_INITIAL_CAPACITY * sizeof(syscall_timer));
    if (array->timers == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
}

static void add_syscall(timer_array *array, syscall_timer syscall) {
    for (uint64_t i = 0; i < array->size; ++i) {
        if (array->timers[i].identifier == syscall.identifier) {
            array->timers[i].calls += 1;
            array->timers[i].useconds += syscall.useconds;
            array->timers[i].errors += syscall.errors;
            return;
        }
    }
    if (array->size >= array->capacity) {
        array->capacity *= 2;
        array->timers =
            realloc(array->timers, array->capacity * sizeof(syscall_timer));
        if (array->timers == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
    }
    syscall.calls              = 1;
    array->timers[array->size] = syscall;
    array->size++;
}

int timer_loop(pid_t child, timer_array *syscalls) {
    int                       status;
    struct user_regs_struct64 regs;
    struct iovec              iov;

    init_array(syscalls);

    check((ptrace(PTRACE_SEIZE, child, NULL,
                  PTRACE_O_TRACESYSGOOD | PTRACE_O_EXITKILL) == -1),
          "PTRACE_SEIZE");
    waitpid(child, &status, 0);
    if (WIFSIGNALED(status))
        signal_exit(status);

    iov.iov_base = &regs;
    iov.iov_len  = sizeof(regs);

    bool          in_syscall     = false;
    bool          first_execve   = true;
    int           syscall_signal = 0;
    syscall_timer current_syscall;
    clock_t       time;
    while (!WIFEXITED(status)) {
        time = clock();
        check(ptrace(PTRACE_SYSCALL, child, NULL, syscall_signal),
              "PTRACE_SYSCALL");
        time = clock() - time;
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
                if (iov.iov_len == sizeof(struct user_regs_struct64)) {
                    current_syscall.identifier =
                        ((struct user_regs_struct64 *)iov.iov_base)->orig_rax;
                    current_syscall.is32bits = false;
                } else {
                    current_syscall.identifier =
                        ((struct user_regs_struct32 *)iov.iov_base)->orig_eax;
                    current_syscall.is32bits = true;
                }
                current_syscall.calls    = 0;
                current_syscall.useconds = 0;
            } else {
                bool status = SYSCALL_SUCCESS;

                current_syscall.useconds = time;
                current_syscall.errors   = 0;
                if (iov.iov_len == sizeof(struct user_regs_struct64)) {
                    long long int syscall_return =
                        ((struct user_regs_struct64 *)iov.iov_base)->rax;
                    if (syscall_return < 0) {
                        current_syscall.errors = 1;
                        status                 = SYSCALL_ERROR;
                    }
                } else {
                    unsigned int syscall_return =
                        ((struct user_regs_struct32 *)iov.iov_base)->eax;
                    if (syscall_return > 0xfffff000) {
                        current_syscall.errors = 1;
                        status                 = SYSCALL_ERROR;
                    }
                }
                add_syscall(syscalls, current_syscall);
                if (first_execve) {
                    if (status == SYSCALL_ERROR)
                        return EXIT_FAILURE;
                    if (iov.iov_len == sizeof(struct user_regs_struct32)) {
                        fprintf(stderr,
                                "[ Process PID=%i runs in 32 bit mode. ]\n",
                                child);
                        free(syscalls->timers);
                        init_array(syscalls);
                    }

                    first_execve = false;
                }
            }
            in_syscall = !in_syscall;
        } else if (WIFSTOPPED(status)) {
            syscall_signal = WSTOPSIG(status);
        }
    }
    return WEXITSTATUS(status);
}

void print_array(timer_array array) {

    uint64_t total_time = array.timers[0].useconds;
    for (uint64_t i = 1; i < array.size; ++i) {
        total_time += array.timers[i].useconds;
        for (uint64_t j = i;
             j > 0 && array.timers[j - 1].useconds < array.timers[j].useconds;
             --j) {
            syscall_timer temp  = array.timers[j];
            array.timers[j]     = array.timers[j - 1];
            array.timers[j - 1] = temp;
        }
    }

    fprintf(stderr,
            "%% time     seconds  usecs/call     calls    errors syscall\n");
    fprintf(stderr, "------ ----------- ----------- --------- --------- "
                    "----------------\n");

    uint64_t total_calls  = 0;
    uint64_t total_errors = 0;
    for (uint64_t i = 0; i < array.size; ++i) {
        syscall_timer timer = array.timers[i];
        char const   *syscall_name;
        if (timer.is32bits) {
            syscall_name = get_syscall_data32(timer.identifier).name;
        } else {
            syscall_name = get_syscall_data64(timer.identifier).name;
        }

        total_calls += timer.calls;
        total_errors += timer.errors;
        fprintf(stderr, "%6.2f %11.6lf %11lu %9lu %9lu %s\n",
                (double)timer.useconds / (double)total_time * 100.,
                (double)timer.useconds / 1000000., timer.useconds / timer.calls,
                timer.calls, timer.errors, syscall_name);
    }
    fprintf(stderr, "------ ----------- ----------- --------- --------- "
                    "----------------\n");
    fprintf(stderr, "100.00 %11.6lf %11lu %9lu %9lu total\n",
            (double)total_time / 1000000, total_time / total_calls, total_calls,
            total_errors);
}
