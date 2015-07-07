#define _GNU_SOURCE


#define open open______ // Get around multiple definitions

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>

int recv(int socket, void* buffer, size_t length, int flags)
{
    return read(socket, buffer, length);
}

int send(int socket, void* buffer, size_t length, int flags)
{
    return write(socket, buffer, length);
}
