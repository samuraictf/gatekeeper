#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>

void kill_timeout(int seconds) {
    pid_t child = fork();

    /**
     * NOTE: We kill from the child so that we do not halt
     *       interactive programs when run from the command-line,
     *       and as a ~useless benefit, we can't be killed via
     *       kill(getppid()).
     */
    if(child == 0) {
        sleep(seconds);
        kill(-child, SIGKILL);
    }
    setpgrp();
}
