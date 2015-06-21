#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <linux/sched.h>

void set_got_nobind() {
    setenv("LD_BIND_NOT", "1", 1);
}
