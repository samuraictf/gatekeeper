#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

void randenv() {
    char data[0x1000];
    int fd = open("/dev/urandom", O_RDONLY);
    int v = 0;
    read(fd, &v, sizeof(v));
    close(fd);
    v &= (sizeof(data)-2);
    memset(data, 'A', v);
    data[v]=0;
    setenv("FUN", data, 1);
}

int main(int argc, char** argv) {
    if(argc < 2) {
        printf("Usage: %s argv0 argv1\n", argv[0]);
        exit(1);
    }
    randenv();
    execvp(argv[1], &argv[1]);
}
