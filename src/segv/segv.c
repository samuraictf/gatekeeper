#define _GNU_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <libgen.h>
#include <time.h>

void log_segv(char* argv0) {
    char *argv0_basename  = basename(argv0);
    char *filename;
    char *ld_preload = "libSegFault.so";
    asprintf(&filename, "%s/segfault-%s-%lu", getenv("TMPDIR"), argv0_basename, (unsigned long) time(0));

    if(getenv("LD_PRELOAD")) {
        asprintf(&ld_preload, "%s:%s", "libSegFault.so", getenv("LD_PRELOAD"));
    }

    setenv("SEGFAULT_USE_ALTSTACK", "1", 1);
    setenv("SEGFAULT_OUTPUT_NAME", filename, 1);
    setenv("LD_PRELOAD", ld_preload, 1);
}
