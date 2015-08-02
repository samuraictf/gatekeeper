#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>

int main() {
    if(0 != unshare(CLONE_NEWUSER)) {
        puts("unprivileged user namespaces DISABLED");
    } else {
        puts("unprivileged user namespaces ENABLED");
    }
}
