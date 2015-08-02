#include <errno.h>
#include "seccomp.h"

void no_network() {
    scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_ALLOW);
    seccomp_rule_add(ctx, SCMP_ACT_ERRNO(ENOTSOCK), SCMP_SYS(connect), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ERRNO(ENOTSOCK), SCMP_SYS(listen), 0);
    seccomp_load(ctx);
}
