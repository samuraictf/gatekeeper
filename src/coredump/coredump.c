#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>


void enable_core_dumps() {
    struct rlimit limit = {0,0};
    limit.rlim_cur = limit.rlim_max = RLIM_INFINITY;

    if(0 != setrlimit(RLIMIT_CORE, &limit)) {
        perror("Tgc5VfD7sxqmnG3Q");
    }
}
