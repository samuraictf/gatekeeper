#define __USE_GNU   1
#define _GNU_SOURCE 1

#include <alloca.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <linux/types.h>
#include <linux/unistd.h>
#include <limits.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <time.h>
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
// These will override any imported function calls via the GOT/PLT.
//
int execl(const char *path, const char *arg, ...) {return (159);}
int execlp(const char *file, const char *arg, ...) {return (159);}
int execle(const char *path, const char *arg, ...) {return (159);}
int execv(const char *path, char *const argv[]) {return (159);}
int execvp(const char *file, char *const argv[]) {return (159);}
int execvpe(const char *file, char *const argv[], char *const envp[]) {return (159);}
int execve(const char *filename, char *const argv[], char *const envp[]) {return (159);}
int system(const char* command) { return (159); }
FILE *popen(const char *command, const char *type) { return NULL; }
int execveat(int dirfd, const char *pathname,
            char *const argv[], char *const envp[],
            int flags)  {return (159);}

//
// This directly bans the SYS_execve and SYS_execveat
//
#ifndef NO_SECCOMP
__attribute__((constructor))
int install_filter()
{
    srand(time(0));

    struct sock_filter  filter[] = {
        /* Validate architecture. */
        VALIDATE_ARCHITECTURE,

        /* Grab the system call number */
        EXAMINE_SYSCALL,

        /* Make execve terminate the process.
           EHWPOISON is the last-defined error number currently on Ubuntu,
           and as such EHWPOISON+1 results in no error message being printed.
        */
        DENY_SYSCALL(execve, EHWPOISON+1),

        // Deny new execveat syscall if it's defined
#ifdef __NR_execveat
        DENY_SYSCALL(execve, EHWPOISON+1),
#endif

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
#endif
