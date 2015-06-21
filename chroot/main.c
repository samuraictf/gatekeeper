#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <linux/sched.h>
#include <signal.h>
#include "chroot.h"

int main(int argc, char** argv) {
    if(argc < 3) {
        printf("Usage: %s <dir> argv0 argv1\n", argv[0]);
        exit(1);
    }

    do_chroot(argv[1]);

    execvp(argv[2], &argv[2]);
}
