#include <stdio.h>
#include <unistd.h>
#include "proxy.h"

int seconds = 0;

callback_rv delay_cb(void* ctx, void** ppbuf, size_t* psize, size_t* palloc)
{
    usleep(1000 * (long) ctx);
    return CB_OKAY;
}

void delay_stdout(long milliseconds)
{
    register_io_callback(STDOUT_FILENO, delay_cb, (void*) milliseconds);
    register_io_callback(STDERR_FILENO, delay_cb, (void*) milliseconds);
}
void delay_stdin(long milliseconds)
{
    register_io_callback(STDIN_FILENO, delay_cb, (void*) milliseconds);
}
