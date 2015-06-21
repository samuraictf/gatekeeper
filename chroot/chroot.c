#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <linux/sched.h>

void do_chroot(char* path) {
    unshare(CLONE_NEWUSER);
    chdir(path);
    chroot(".");
}
