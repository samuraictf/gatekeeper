#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>

void kill_timeout(int seconds) {
    if(fork() == 0) {
        sleep(seconds);
        kill(getppid(), SIGKILL);
    }
}
