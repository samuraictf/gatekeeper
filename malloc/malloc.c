#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <linux/sched.h>

void set_malloc_flags() {
    setenv("MALLOC_PERTURB_", "127", 1);
    setenv("MALLOC_CHECK_", "2", 1);
}
