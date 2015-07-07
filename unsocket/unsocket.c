#include <unistd.h>

int recv(int socket, void* buffer, size_t length, int flags)
{
    return read(socket, buffer, length);
}

int send(int socket, void* buffer, size_t length, int flags)
{
    return write(socket, buffer, length);
}
