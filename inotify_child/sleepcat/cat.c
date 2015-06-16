#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char** argv) {
    char buffer[512];
    puts("Sleeping...");
    sleep(1);
    int fd = open(argv[1], O_RDONLY);
    ssize_t n = read(fd, buffer, sizeof(buffer));
    write(1, buffer, n);
}
