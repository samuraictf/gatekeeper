#include <stdio.h>
#include <unistd.h>
#include "proxy.h"

int seconds = 0;

void delay_cb(int fd, void* ctx, void** ppbuf, size_t* psize, size_t* palloc)
{
    usleep(1000 * (long) ctx);
}

void delay_stdout(long milliseconds)
{
    proxy_register_callback(STDOUT_FILENO, delay_cb, (void*) milliseconds);
    proxy_register_callback(STDERR_FILENO, delay_cb, (void*) milliseconds);
}
void delay_stdin(long milliseconds)
{
    proxy_register_callback(STDIN_FILENO, delay_cb, (void*) milliseconds);
}
