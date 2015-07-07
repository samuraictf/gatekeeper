#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

#include "regex.h"
#include "proxy.h"

void callback(char* buffer, size_t buffer_size, char* match)
{
    kill(regex_child_pid, SIGKILL);
    puts("Sorry, Dave.  I can't allow you to do that.");
    exit(-1);
}

int main(int argc, char** argv) {
    if(argc < 3) {
        printf("Usage: %s <file> argv0 argv1\n", argv[0]);
        exit(1);
    }

    regex_filter_stdio(STDOUT_FILENO, argv[1], callback);

    regex_fork_execvp(&argv[2]);

    regex_pump();
}
