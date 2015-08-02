#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <asm/unistd.h>
#include "no_execve.h"

int main(int argc, char** argv) {
    if(argc < 2) {
        printf("Usage: %s argv0 argv1\n", argv[0]);
        exit(1);
    }

    printf("__NR_execve = %i\n", __NR_execve);

    no_execve();

    if(0 != execvp(argv[1], &argv[1])) {
        perror("execvp");
    }
}
