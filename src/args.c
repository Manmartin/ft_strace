#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ft_strace.h"

static void print_usage() {
    fprintf(
        stderr,
        "Usage: ft_strace    PROG [ARGS]\n   or: ft_strace -c PROG [ARGS]\n");
}

// TODO: implement path resolution
static char *resolve_path(char *command) { return command; }

void verify_args(args_t *args, int argc, char **argv, char **env) {
    size_t current_arg = 1;
    memset(args, 0, sizeof(args_t));
    if (argc < 2) {
        fprintf(stderr, "ft_strace: no program supplied.\n\n");
        print_usage();
        exit(EXIT_FAILURE);
    }

    if (strncmp("-c", argv[current_arg], 3) == 0) {
        args->timer_mode = true;
        ++current_arg;
    }
    if (current_arg == (size_t)argc) {
        fprintf(stderr, "ft_strace: no program supplied.\n\n");
        print_usage();
        exit(EXIT_FAILURE);
    }

    args->program_path = resolve_path(argv[current_arg]);
    args->args         = &argv[current_arg];
    args->env          = env;
}
