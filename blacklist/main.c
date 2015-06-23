#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>

#include "blacklist.h"

int main(int main, char** argv) {
    blacklist_range("127.0.0.0", "127.255.255.255");
    blacklist_range("::1", "::1");

    if(blacklist_check_stdio()) {
        puts("No connections from localhost");
    } else {
        puts("Howdy, stranger!");
    }
}
