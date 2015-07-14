#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "proxy.h"

void replace(char* data, size_t size, char a, char b) {
    for(size_t i = 0; i < size; i++) {
        if(data[i] == a)
            data[i] = b;
    }
}

void data_in(int fd, void* ctx, void** ppbuf, size_t* psize, size_t* palloc)
{
    replace(*ppbuf, *psize, 'A', 'a');
}

void data_out(int fd, void* ctx, void** ppbuf, size_t* psize, size_t* palloc)
{
    replace(*ppbuf, *psize, 'B', 'b');
}

void data_out2(int fd, void* ctx, void** ppbuf, size_t* psize, size_t* palloc)
{
    replace(*ppbuf, *psize, 'C', 'c');
}


void data_err(int fd, void* ctx, void** ppbuf, size_t* psize, size_t* palloc)
{
    replace(*ppbuf, *psize, 'D', 'd');
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
