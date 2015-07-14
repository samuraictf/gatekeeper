#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>


int main(int argc, char** argv) {
    if(argc < 2) {
        printf("Usage: %s <pgid> argv0 argv1\n", argv[0]);
        exit(1);
    }
    setpgid(0, atoi(argv[1]));
    execvp(argv[2], &argv[2]);
}
