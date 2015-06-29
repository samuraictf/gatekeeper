#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>

#include "blacklist.h"

int main() {
    struct ifaddrs* addrs = 0;
    if(0 != getifaddrs(&addrs)) {
        perror("getifaddrs");
        return 1;
    }

    while(addrs != 0) {
        blacklist_sockaddr(addrs->ifa_addr);
        addrs = addrs->ifa_next;
    }

    blacklist_range("127.0.0.0","127.255.255.255");
}
