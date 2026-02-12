#include <bits/types/siginfo_t.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#define __USE_GNU // sigabbrev_np
#include <string.h>

static char *get_si_code(int si_code) {
    switch (si_code) {
    case SI_ASYNCNL:
        return "SI_ASYNCNL";
    case SI_DETHREAD:
        return "SI_DETHREAD";
    case SI_TKILL:
        return "SI_TKILL";
    case SI_SIGIO:
        return "SI_SIGIO";
    case SI_ASYNCIO:
        return "SI_ASYNCIO";
    case SI_MESGQ:
        return "SI_MESGQ";
    case SI_TIMER:
        return "SI_TIMER";
    case SI_QUEUE:
        return "SI_QUEUE";
    case SI_USER:
        return "SI_USER";
    case SI_KERNEL:
        return "SI_KERNEL";
    default:
        return "UNKNOWN";
    }
}

void signal_exit(int status) {
    if (WCOREDUMP(status))
        fprintf(stderr, "Quit (core dumped)\n");
    else
        fprintf(stderr, "+++ killed by SIG%s +++\n",
                sigabbrev_np(WTERMSIG(status)));
    kill(getpid(), WTERMSIG(status));
    exit(status | 0x80);
}

void print_signal(siginfo_t signal) {
    (void)signal;
    fprintf(stderr, "--- ");
    fprintf(stderr, "SIG%s {si_signo=SIG%s, si_code=%s si_pid=%i, si_uid=%i}",
            sigabbrev_np(signal.si_signo), sigabbrev_np(signal.si_signo),
            get_si_code(signal.si_code), signal.si_pid, signal.si_uid);
    fprintf(stderr, " ---\n");
}
