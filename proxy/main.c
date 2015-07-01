#include <stdio.h>
#include <string.h>
#include "proxy.h"

void data_in(int fd, void* ctx, void** ppbuf, size_t* psize, size_t* palloc)
{
    char* data = *ppbuf;
    while((data = strstr(data, "aaaaa"))) {
        memcpy(data, "AAAAA", 5);
    }
}

void data_out(int fd, void* ctx, void** ppbuf, size_t* psize, size_t* palloc)
{
    char* data = *ppbuf;
    while((data = strstr(data, "bbbbb"))) {
        memcpy(data, "BBBBB", 5);
    }
}

void data_out2(int fd, void* ctx, void** ppbuf, size_t* psize, size_t* palloc)
{
    char* data = *ppbuf;
    while((data = strstr(data, "ccccc"))) {
        memcpy(data, "CCCCC", 5);
    }
}


void data_err(int fd, void* ctx, void** ppbuf, size_t* psize, size_t* palloc)
{
    char* data = *ppbuf;
    while((data = strstr(data, "ddddd"))) {
        memcpy(data, "DDDDD", 5);
    }
}


int main(int argc, char** argv) {
    if(argc < 2) {
        printf("Usage: %s argv0 argv1\n", argv[0]);
        exit(1);
    }

    proxy_register_callback(0, data_in, 0);
    proxy_register_callback(1, data_out, 0);
    proxy_register_callback(1, data_out2, 0);
    proxy_register_callback(2, data_err, 0);
    proxy_fork_execvp(&argv[1]);
    proxy_pump();
}
