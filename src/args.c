#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ft_strace.h"

static void print_usage(void) {
    fprintf(
        stderr,
        "Usage: ft_strace    PROG [ARGS]\n   or: ft_strace -c PROG [ARGS]\n");
}

static char *resolve_path(char *command) {
    // check if command is a route
    if (strchr(command, '/'))
        return strdup(command);
    char *path = getenv("PATH");
    if (path == NULL)
        return NULL;

    int command_len = strlen(command);
    int dir_len;
    for (char *current_dir = path; *current_dir != '\0';) {

        dir_len            = strcspn(current_dir, ":");
        char *command_path = calloc(command_len + dir_len + 2, sizeof(char));
        if (command_path == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }

        memmove(command_path, current_dir, dir_len);
        command_path[dir_len] = '/';
        strcat(command_path, command);
        struct stat stats;
        if (stat(command_path, &stats) == 0)
            return command_path;

        free(command_path);
        current_dir += dir_len;
        if (*current_dir != '\0')
            ++current_dir;
    }
    return NULL;
}

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

    if ((args->program_path = resolve_path(argv[current_arg])) == NULL) {
        fprintf(stderr, "ft_strace: executable '%s' not found.\n",
                argv[current_arg]);
        exit(EXIT_FAILURE);
    }
    args->args = &argv[current_arg];
    args->env  = env;
}
