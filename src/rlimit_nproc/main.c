#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>

#include <sys/time.h>
#include <sys/resource.h>

#include "rlimit_nproc.h"

int main(int argc, char** argv) {
    if(argc < 3) {
        printf("Usage: %s <limit> argv0 argv1\n", argv[0]);
        exit(1);
    }
    rlimit_nproc(atoi(argv[1]));
    execvp(argv[2], &argv[2]);
}
