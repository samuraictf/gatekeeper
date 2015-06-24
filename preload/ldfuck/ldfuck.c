#define _GNU_SOURCE 1
#include <dlfcn.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <elf.h>
#include <link.h>
#include <string.h>
#include <ctype.h>
#include <sys/mman.h>

void *dlopen(const char *filename, int flag);


size_t offsets[] = {
#if defined(__i386__)
    0x174
#elif defined(__amd64__)
    0x270, 0x174,
#elif defined(__arm__)
    0x174
#elif defined(__arch64__)
    0x238
#endif
};

char hash[] = {"\x04\xf1\x86\x29\xef\x42\xb0\x62\xed\x0c\x8f\x60\xd5\xbf\xaa\x40\xa7\xd2\x8e\xf7"};

__attribute__((constructor))
void fuck_up_link_map(int argc, char** argv, char** envp) {
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
            //
            // Destory the name of the binary.  This prevents using
            // the link_map to find base addresses.
            //
            // dprintf(2, "Killed linkmap name: %s\n", linkmap->l_name);
            linkmap->l_name = "";


            //
            // Destroy the BUILD_ID in the elf with a different one.
            //
            char* address = (char*) linkmap->l_addr;

            for(size_t i = 0; i < sizeof(offsets)/sizeof(offsets[0]); i++) {
                char* build_id = address + offsets[i];
                if(0 == memcmp(build_id + 0xc, "GNU\x00", 4)) {
                    // All of the offsets are less than a page from the bases
                    mprotect(address, 0x1000, PROT_READ | PROT_WRITE);
                    memcpy(build_id + 0x10, hash, sizeof(hash));
                    mprotect(address, 0x1000, PROT_READ);
                }
            }
        }

        linkmap = linkmap->l_next;
    }
}
