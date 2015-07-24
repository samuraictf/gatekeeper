#define _GNU_SOURCE

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>

int qemu_detected = 0;

// This will crash gdb-7.9.x via infinite recursion
__attribute__ ((constructor))
void _ZN5Utils9transformIPN15ProjectExplorer13BuildStepListEZNKS1_18BuildConfiguration14knownStepListsEvEUlS3_E_EEN4Myns5QListIDTclfp0_cvT__EEEEERKNS7_IS8_EET0_()
{ }

// This will crash gdb-7.7.1 (Ubuntu 14.04) via SIGSEGV
__attribute__ ((constructor))
void _ZNVSt9__ft88888B8888888888888888omi()
{ }

__attribute__ ((constructor))
void anti_ptrace(void)
{
    pid_t child;

    _ZNVSt9__ft88888B8888888888888888omi();
    _ZN5Utils9transformIPN15ProjectExplorer13BuildStepListEZNKS1_18BuildConfiguration14knownStepListsEvEUlS3_E_EEN4Myns5QListIDTclfp0_cvT__EEEEERKNS7_IS8_EET0_();

    //
    // Disallow loading any modules via LD_PRELOAD.
    //
    // Ideally, everything will be statically linked so we don't
    // have to worry about this.
    //
    if(getenv("LD_PRELOAD"))
        exit(1);

    //
    // Outright crash strace
    //
    size_t buffer_size = 0x10000;
    char *select_buffer = malloc(buffer_size);
    memset(select_buffer, 0xff, buffer_size);
    select_buffer[0] = 0xf8; // Skip over fd 0-2
    select(0xdeadbeef, (fd_set *)select_buffer, NULL, NULL, NULL);
    free(select_buffer);

    //
    // Detect if we are currently being ptraced by a different naive
    // ptracer, like ltrace.
    //
    child = fork();
    if (child > 0) {
        int err = ptrace(PTRACE_ATTACH, child, 0, 0);

        kill(child, SIGKILL);

        if (err < 0)
            exit(1);
    }
    else if (child == 0) {
        sleep(10);
        exit(0);
    }

    //
    // Detect if we are being run under qemu-user emulation, which could
    // be used with a remote GDB stub and other things to perform the debugging.
    //
    // Normally, this would return EFAULT but qemu-user always returns ENOTDIR.
    //
    int code = syscall(SYS__sysctl, 1);

    if(code == ENOTDIR) {
        qemu_detected = 1;
    }
}

