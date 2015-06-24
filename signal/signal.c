#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <linux/sched.h>
#include <signal.h>

#ifndef SIG_SETMASK
#define SIG_SETMASK   2      /* Set the set of blocked signals.  */
#endif

int sigfillset(sigset_t *set);
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);

void ignore_all_signals() {
    sigset_t signal_set;
    sigfillset(&signal_set);
    sigprocmask(SIG_SETMASK, &signal_set, NULL);
}
