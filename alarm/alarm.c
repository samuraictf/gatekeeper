#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int main(int argc, char** argv) {
    if(argc < 3) {
        printf("Usage: %s <seconds> argv0 argv1\n", argv[0]);
        exit(1);
    }

    alarm(atoi(argv[1]));
    execvp(argv[2], &argv[2]);
}
