#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>


void set_malloc_flags() {
#ifndef __APPLE__
    setenv("MALLOC_PERTURB_", "85", 1);       // Data is overwritten
    setenv("MALLOC_CHECK_", "2", 1);          // Enable some internal checks
    setenv("MALLOC_MMAP_THRESHOLD_", "1", 1); // Always use mmap()
#else
    // setenv("DYLD_INSERT_LIBRARIES", "/usr/lib/libgmalloc.dylib", 1);
    setenv("MallocGuardEdges", "1", 1);
    setenv("MallocScribble", "1", 1);
    setenv("MALLOC_PROTECT_BEFORE", "1", 1);
#endif
}
