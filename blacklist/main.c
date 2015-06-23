#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>

#include "blacklist.h"

int main(int argc, char** argv) {
    if(argc < 2) {
        printf("Usage: %s argv0 argv1\n", argv[0]);
        exit(1);
    }

    blacklist_parse("127.0.0.0-127.255.255.255");
    blacklist_range("::1", "::1");

    if(blacklist_check_stdio()) {
        puts("No connections from localhost");
        exit(1);
    }

    execvp(argv[1], &argv[1]);
}
