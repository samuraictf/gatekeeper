#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/capability.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/prctl.h>

void capdrop() {
    // Disable ever gaining root e.g. via setuid
    prctl(PR_SET_SECUREBITS, 0x2f);

    // Drop all capabilities from the possible set
    for(int cap = 0; 0 == prctl(PR_CAPBSET_DROP, cap, 0, 0, 0); cap++);

    // Drop all capabilities that we have
    struct __user_cap_header_struct hdr = {0};
    struct __user_cap_data_struct data = {0};
    hdr.version = _LINUX_CAPABILITY_VERSION;
    hdr.pid = 0;
    data.effective = data.permitted = data.inheritable = 0;
    capset(&hdr, &data);

    // Not sure what this does
    prctl(PR_SET_KEEPCAPS, 0, 0, 0, 0);

    // Disable core dumps
    prctl(PR_SET_DUMPABLE, 0, 0, 0, 0);

    // Disable all setuid effects
    prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0);
}
