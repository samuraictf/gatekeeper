#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <linux/sched.h>

int main(int argc, char** argv) {
    if(argc < 3) {
        printf("Usage: %s <name> argv0 argv1\n", argv[0]);
        exit(1);
    }
    char *filename;
    char *ld_preload = "libSegFault.so";
    asprintf(&filename, "%s/segfault-%s-%i", getenv("TMPDIR"), argv[1], time(0));

    if(getenv("LD_PRELOAD")) {
        asprintf(&ld_preload, "%s:%s", "libSegFault.so", getenv("LD_PRELOAD"));
    }

    setenv("SEGFAULT_USE_ALTSTACK", "1", 1);
    setenv("SEGFAULT_OUTPUT_NAME", filename, 1);
    setenv("LD_PRELOAD", ld_preload, 1);

    execvp(argv[2], &argv[2]);
}
