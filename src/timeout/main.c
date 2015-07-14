#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "timeout.h"

int main(int argc, char** argv) {
    if(argc < 3) {
        printf("Usage: %s <seconds> argv0 argv1\n", argv[0]);
        exit(1);
    }

    kill_timeout(atoi(argv[1]));

    return execvp(argv[2], &argv[2]);
}
