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

void do_capdrop() {
    struct __user_cap_header_struct hdr = {0};
    struct __user_cap_data_struct data = {0};
    hdr.version = _LINUX_CAPABILITY_VERSION;
    hdr.pid = 0;
    data.effective = data.permitted = data.inheritable = 0;
    capset(&hdr, &data);
    prctl(PR_SET_KEEPCAPS, 0);
    prctl(PR_SET_DUMPABLE, 0);
}
