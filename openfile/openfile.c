#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <fcntl.h>              /* Obtain O_* constant definitions */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int main(int argc, char** argv)
{
    if(argc < 4) {
        dprintf(2, "usage: %s <file> <fd #> arg0 arg1\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_WRONLY);

    if(fd < 1) {
        dprintf(2, "could not open %s: %s\n", argv[1], strerror(errno));
        return 1;
    }

    dup2(fd, atoi(argv[2]));
    close(fd);
    execvp(argv[3], &argv[3]);
}