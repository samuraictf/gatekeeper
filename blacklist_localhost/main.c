#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "blacklist.h"

int main(int argc, char** argv) {
    if(argc < 2) {
        printf("Usage: %s argv0 argv1\n", argv[0]);
        exit(1);
    }

    struct ifaddrs* addrs = 0;
    if(0 != getifaddrs(&addrs)) {
        perror("getifaddrs");
        return 1;
    }

    while(addrs != 0) {
        blacklist_sockaddr(addrs->ifa_addr);
        addrs = addrs->ifa_next;
    }

    if(blacklist_check_stdio()) {
        puts("No connections from localhost");
        exit(1);
    }

    execvp(argv[1], &argv[1]);
}
