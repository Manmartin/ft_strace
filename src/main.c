#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ft_strace.h"

int main(int argc, char **argv, char **env) {
    args_t args;
    verify_args(&args, argc, argv, env);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }
    if (pid == 0) {
        kill(getpid(), SIGSTOP);
        execve(args.program_path, args.args, args.env);
        perror("ft_strace: execve");
        fprintf(stderr, "+++ exited with %i +++\n", EXIT_FAILURE);
        exit(EXIT_FAILURE);
    }
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    free(args.program_path);

    struct sigaction act;
    memset(&act, 0, sizeof(act));

    act.sa_handler = SIG_IGN;
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGQUIT, &act, NULL);
    sigaction(SIGTSTP, &act, NULL);
    return trace_loop(pid);
}
