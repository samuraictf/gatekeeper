#define _GNU_SOURCE


#define open open______ // Get around multiple definitions

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <dlfcn.h>

#undef open


typedef int (*p_open)(const char*, int, mode_t);

p_open real_open;

int open(const char *pathname, int flags, mode_t mode)
{
    if(0 == strcmp(pathname, "/dev/ctf"))
        return 1023;

    return real_open(pathname, flags, mode);
}

__attribute__((constructor))
static int
initialize()
{
    real_open = (p_open) dlsym(RTLD_NEXT, "open");
    return 0;
}
