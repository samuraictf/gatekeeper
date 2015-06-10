#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <linux/sched.h>

int main(int argc, char** argv) {
    if(argc < 2) {
        printf("Usage: %s <dir> argv0 argv1\n", argv[0]);
        exit(1);
    }
    unshare(CLONE_NEWUSER);
    chdir(argv[1]);
    chroot(".");
    execvp(argv[2], &argv[2]);
}