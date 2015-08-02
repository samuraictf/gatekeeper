#define __USE_GNU   1
#define _GNU_SOURCE 1
#include <errno.h>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <unistd.h>
#include "seccomp-bpf.h"
#include "regs.h"

#ifndef __NR_syscalls
#if defined(__i386__)
#define __NR_syscalls __NR_prlimit64+1
#elif defined(__amd64__)
#define __NR_syscalls __NR_prlimit64+1
#elif defined(__arm__)
#define __NR_syscalls __NR_timerfd_settime+1
#endif
#endif


//
// This directly bans the SYS_execve and SYS_execveat
//
int install_child_control()
{
    struct sock_filter  filter[] = {
        /* Validate architecture. */
        VALIDATE_ARCHITECTURE,

        /* Grab the system call number */
        EXAMINE_SYSCALL,

        /* Disallow invocation of setpgrp, which makes it harder to kill processes */
        DENY_SYSCALL(setpgid, EHWPOISON+1),

        /* Disallow invocation of setsid, which makes it harder to kill processes */
        DENY_SYSCALL(setsid, EHWPOISON+1),

        /* Disallow invocation of kill, so it can't kill our things */
        DENY_SYSCALL(kill, EHWPOISON+1),

        // Deny any undefined syscalls. Will show up as error
        // 'invalid system call'.
        BPF_JUMP(BPF_JMP + BPF_JGE + BPF_K, __NR_syscalls, 0, 1),
        BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_KILL),

        /* Allow everything else */
        ALLOW_PROCESS
    };
    struct sock_fprog   prog = {
        .len    = (unsigned short)(sizeof(filter) / sizeof(filter[0])),
        .filter = filter,
    };
    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0, 0))
    {
        return 1;
    }
    if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, (unsigned long) &prog, 0, 0, 0))
    {
        return 1;
    }

    return 0;
}


