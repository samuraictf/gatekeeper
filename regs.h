#include <linux/unistd.h>
#include <unistd.h>

#define syscall_arg(_n) (offsetof(struct seccomp_data, args[_n]))
#define syscall_nr (offsetof(struct seccomp_data, nr))

#if defined(__i386__)
#define REG_RESULT  gregs[REG_EAX]
#define REG_SYSCALL gregs[REG_EAX]
#define REG_ARG0    gregs[REG_EBX]
#define REG_ARG1    gregs[REG_ECX]
#define REG_ARG2    gregs[REG_EDX]
#define REG_ARG3    gregs[REG_ESI]
#define REG_ARG4    gregs[REG_EDI]
#define REG_ARG5    gregs[REG_EBP]
#elif defined(__x86_64__)
#define REG_RESULT  gregs[REG_RAX]
#define REG_SYSCALL gregs[REG_RAX]
#define REG_ARG0    gregs[REG_RDI]
#define REG_ARG1    gregs[REG_RSI]
#define REG_ARG2    gregs[REG_RDX]
#define REG_ARG3    gregs[REG_R10]
#define REG_ARG4    gregs[REG_R8]
#define REG_ARG5    gregs[REG_R9]
#elif defined(__arm__)
#define REG_RESULT  arm_r0
#define REG_SYSCALL arm_r7
#define REG_ARG0    arm_r0
#define REG_ARG1    arm_r1
#define REG_ARG2    arm_r2
#define REG_ARG3    arm_r3
#else
#error "Unknown Architecture"
#endif
