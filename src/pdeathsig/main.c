#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>

#include "pdeathsig.h"

int main(int argc, char** argv) {
    if(argc < 2) {
        printf("Usage: %s <dir> argv0 argv1\n", argv[0]);
        exit(1);
    }

    set_pdeathsig(SIGKILL);

    execvp(argv[1], &argv[1]);
}
