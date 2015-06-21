#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <linux/sched.h>
#include <signal.h>
#include "signal.h"

int main(int argc, char** argv) {
    if(argc < 2) {
        printf("Usage: %s argv0 argv1\n", argv[0]);
        exit(1);
    }

    ignore_all_signals();

    execvp(argv[1], &argv[1]);
}
