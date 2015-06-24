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
#include <time.h>

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

void shuffle(void **array, size_t n)
{
    if (n > 1)
    {
        size_t i;
        for (i = 0; i < n - 1; i++)
        {
          size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
          void* t = array[j];
          array[j] = array[i];
          array[i] = t;
        }
    }
}

__attribute__((constructor))
void fuck_up_link_map(int argc, char** argv, char** envp) {
    void* handle = dlopen(NULL, RTLD_LAZY);
    struct link_map * lm_libc, * lm_other;
    struct link_map * linkmap = 0;

    if(0 == handle || -1 == dlinfo(handle, RTLD_DI_LINKMAP, &linkmap)) {
        puts("g2nb2nzGgMmsHWij");
        exit(1);
    }

    struct link_map* map = linkmap;
    char* preload = getenv("LD_PRELOAD");

    //
    // Perform basic modifications to the link map
    //
    size_t n_linkmaps = 0;
    size_t idx_libc = 0;
    while(map) {
#ifdef DEBUG
        dprintf(2, "%p: %s\n", (void*)map->l_addr, map->l_name);
#endif

        //
        // Remove ourselves from the list and destroy BUILD_IDs
        //
        if(preload
          && map->l_name
          && strlen(map->l_name)
          && strstr(preload, map->l_name)) {
            map->l_name[0] = 0;
        }

        //
        // Destroy the BUILD_ID in the elf with a different one.
        //
        char* address = (char*) map->l_addr;

        if(address && strlen(map->l_name)) {
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

        // Keep track of where libc is
        if(strstr(map->l_name, "/libc.so")) {
            idx_libc = n_linkmaps;
        }

        n_linkmaps++;
        map = map->l_next;
    }


    // Get them all into a list to be shuffled
    size_t n = 0;
    struct link_map** all_maps = calloc(sizeof(void*), n_linkmaps);
    char ** map_names = calloc(sizeof(void*), n_linkmaps);
    for(map = linkmap; map; map=map->l_next, n++) {
        all_maps[n] = map;
        map_names[n] = map->l_name;
    }

    // Shuffle all except the first two, which must come first.
    // Keep shuffling as long as the libc libname doesn't move.
    srand(time(0) + getpid());
    while(all_maps[idx_libc]->l_name
        && strlen(all_maps[idx_libc]->l_name)
        && 0 != strstr(all_maps[idx_libc]->l_name, "/libc.so")) {
        shuffle((void**) &all_maps[2], n_linkmaps-2);
    }

    // Update the forward/back links to reflect the new ordering.
    // Update the names to be in the original order
    for(size_t i = 0; i < n_linkmaps; i++) {
        if(i == 0)
            all_maps[i]->l_prev = 0;
        else
            all_maps[i]->l_prev = all_maps[i-1];

        if(i + 1 == n_linkmaps)
            all_maps[i]->l_next = 0;
        else
            all_maps[i]->l_next = all_maps[i+1];

        all_maps[i]->l_name = map_names[i];
    }
}
