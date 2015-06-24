#define _GNU_SOURCE 1
#include <dlfcn.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <elf.h>
#include <link.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

__attribute__((constructor))
void one_true_path(int argc, char** argv, char** envp) {
    char* current = realpath("/proc/self/exe", 0);
    char* expected = getenv("ONE_TRUE_PATH");

    if(expected && strcmp(expected, current)) {
        kill(getppid(), SIGKILL);
        _exit(0);
    }
}
