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

#define SECURE_ALL_BITS         0x15
#define SECURE_ALL_LOCKS        (SECURE_ALL_BITS << 1)

void capdrop() {
    // Disable ever gaining root e.g. via setuid
    prctl(PR_SET_SECUREBITS,(SECURE_ALL_BITS | SECURE_ALL_LOCKS));

    // Drop all capabilities from the possible set
    for(int cap = 0; 0 == prctl(PR_CAPBSET_DROP, cap, 0, 0, 0); cap++);

    // Drop all capabilities that we have
    struct __user_cap_header_struct hdr = {0};
    struct __user_cap_data_struct data = {0};
    hdr.version = _LINUX_CAPABILITY_VERSION;
    hdr.pid = 0;
    data.effective = data.permitted = data.inheritable = 0;
    capset(&hdr, &data);

    prctl(PR_SET_KEEPCAPS, 1);
    prctl(PR_SET_DUMPABLE, 0);
    prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0);
}
