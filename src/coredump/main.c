#define _GNU_SOURCE
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "coredump.h"

int main(int argc, char** argv) {
    enable_core_dumps();

    if(fork() == 0) {
        char* x = 0;
        *x = 1;
    }

    wait(NULL);
}
