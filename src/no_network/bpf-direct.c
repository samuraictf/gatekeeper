#ifndef __APPLE__
#define __USE_GNU   1
#define _GNU_SOURCE 1

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
#include "seccomp-bpf.h"
#include "regs.h"
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>

#define _SYSCALL_H
#undef __ASSEMBLER__

#ifndef SYS_socketcall_socket
#define SYS_socketcall_socket 1
#endif

int
disallow_socketcall
(
    void
)
{
    struct sock_filter  filter[] = {
        /* Validate architecture. */
        VALIDATE_ARCHITECTURE,

        /* Grab the system call number */
        BPF_STMT(BPF_LD + BPF_W + BPF_ABS, syscall_nr),


        /* Kill all socketcall, except PF_LOCAL*/
#if   defined(__x86_64__)
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_socket, 0, 3),
        BPF_STMT(BPF_LD + BPF_W + BPF_ABS, syscall_arg(0)),
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, (unsigned long) PF_LOCAL, 1, 0),
        BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_KILL),
#elif defined(__arm__)
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_socket, 0, 3),
        BPF_STMT(BPF_LD + BPF_W + BPF_ABS, syscall_arg(0)),
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, (unsigned long) PF_LOCAL, 1, 0),
        BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_KILL),

        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_socketcall, 0, 3),
        BPF_STMT(BPF_LD + BPF_W + BPF_ABS, syscall_arg(0)),
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, (unsigned long) SYS_socketcall_socket, 1, 0),
        BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_KILL),
#elif defined(__i386__)
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_socketcall, 0, 3),
        BPF_STMT(BPF_LD + BPF_W + BPF_ABS, syscall_arg(0)),
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, (unsigned long) SYS_socketcall_socket, 1, 0),
        BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_KILL),
#endif

        /* Allow everything else */
        BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_ALLOW)
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


int
main
(
    int     argc,
    char    ** argv,
    char    ** envp
)
{
    if(argc < 2) {
        printf("usage: %s argv0 argv1", argv[0]);
    }
    disallow_socketcall();
    execvp(argv[1],&argv[1]);
}
#else
#include <stdio.h>
int main()
{
    puts("seccomp does not work on OSX");
}
#endif
