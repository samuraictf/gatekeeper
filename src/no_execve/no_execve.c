#include <stdio.h>
#include "seccomp.h"

void no_execve() {
    scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_ALLOW);
    seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(execve), 0);
    printf("syscall: %i\n", SCMP_SYS(execve));
    seccomp_load(ctx);
}
