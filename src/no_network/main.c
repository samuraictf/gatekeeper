#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "no_network.h"

int main(int argc, char** argv) {
    if(argc < 2) {
        printf("Usage: %s argv0 argv1\n", argv[0]);
        exit(1);
    }

    no_network();

    if(0 != execvp(argv[1], &argv[1])) {
        perror("execvp");
    }
}
