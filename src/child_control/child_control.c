#include <stdio.h>
#include <errno.h>
#include <seccomp.h>

void install_child_control() {
    scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_ALLOW);
    seccomp_rule_add(ctx, SCMP_ACT_ERRNO(EHWPOISON+1), SCMP_SYS(setpgid), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ERRNO(EHWPOISON+1), SCMP_SYS(setsid), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ERRNO(EHWPOISON+1), SCMP_SYS(kill), 0);
    seccomp_load(ctx);
}
