#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>

#include <sys/time.h>
#include <sys/resource.h>

void rlimit_nproc(int n) {
    struct rlimit limit = {0,0};
    limit.rlim_cur = limit.rlim_max = n;
    setrlimit(RLIMIT_NPROC, &limit);
}
