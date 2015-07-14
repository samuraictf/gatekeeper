#define _GNU_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>

#ifdef __linux__
#include <linux/sched.h>

void do_chroot(char* path) {
    unshare(CLONE_NEWUSER);
    chdir(path);
    chroot(".");
}
#else
void do_chroot(char* path) {
    puts("MKRWyXUSvQHqLacu");
};
#endif
