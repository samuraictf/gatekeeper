#define _GNU_SOURCE 1
#include <dlfcn.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <elf.h>
#include <link.h>
#include <string.h>

void *dlopen(const char *filename, int flag);

__attribute__((constructor))
void fuck_up_link_map(int argc, char** argv, char** envp) {
    if(0 == getenv("LD_BIND_NOW")) {
        puts("f4IMfQEOAMI3U6SN");
        exit(1);
    }

    void* handle = dlopen(NULL, RTLD_LAZY);
    struct link_map * linkmap = 0;

    if(0 == handle || -1 == dlinfo(handle, RTLD_DI_LINKMAP, &linkmap)) {
        puts("g2nb2nzGgMmsHWij");
        exit(1);
    }

    //
    // We're specifically looking to stop resolution of symbols within
    // libc.  As a result, we want to make sure that anything scanning
    // the link_map list cannot locate anything with this name.
    //
    while(linkmap) {
        if(linkmap->l_name && strstr(linkmap->l_name, "/libc.so")) {
            linkmap->l_name = "";
        }
        linkmap = linkmap->l_next;
    }
}
