#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <fcntl.h>              /* Obtain O_* constant definitions */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char** argv) {
    if(argc < 2) {
        printf("Usage: %s argv0 argv1\n", argv[0]);
        exit(1);
    }

    int first_child, second_child;
    int fds[2];

    pipe2(fds, O_CLOEXEC);

    first_child = fork();

    if(first_child == 0) {
        second_child = fork();
        if(second_child != 0) {
            write(fds[0], &second_child, sizeof(second_child));
            exit(0);
        } else {
            execvp(argv[1], &argv[1]);
            exit(1);
        }
    }

    read(fds[1], &second_child, sizeof(second_child));
    waitpid(second_child, NULL, 0);
    kill(second_child, SIGKILL);
}
