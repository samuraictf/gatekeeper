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

    setenv("MALLOC_PERTURB_", "127", 1);
    setenv("MALLOC_CHECK_", "2", 1);

    execvp(argv[1], &argv[1]);
}