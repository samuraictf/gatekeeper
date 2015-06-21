#define _GNU_SOURCE 1
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <linux/sched.h>
#include <signal.h>

#ifndef SIG_SETMASK
#define SIG_SETMASK   2      /* Set the set of blocked signals.  */
#endif

void ignore_all_signals() {
    sigset_t signal_set;
    sigfillset(&signal_set);
    sigprocmask(SIG_SETMASK, &signal_set, NULL);
}
