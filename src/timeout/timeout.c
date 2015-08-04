#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>

void kill_timeout(int seconds) {
    pid_t child = fork();
    if(child != 0) {
        sleep(seconds);
        kill(-child, SIGKILL);
    }
    setpgrp();
}
