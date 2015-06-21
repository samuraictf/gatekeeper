#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <linux/sched.h>

void set_bind_now() {
    setenv("LD_BIND_NOW", "1", 1);
}
