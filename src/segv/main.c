#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "segv.h"

int main(int argc, char** argv) {
    if(argc < 2) {
        printf("Usage: %s argv0 argv1\n", argv[0]);
        exit(1);
    }
    

    log_segv(argv[1]);

    execvp(argv[1], &argv[1]);
}
