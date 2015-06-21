#include <stdio.h>
#include "proxy.h"

callback_rv data_in(void* ctx, void** ppbuf, size_t* psize, size_t* palloc)
{
    puts("<STDIN>");
    return CB_OKAY;
}

callback_rv data_out(void* ctx, void** ppbuf, size_t* psize, size_t* palloc)
{
    puts("<STDOUT>");
    return CB_OKAY;
}

callback_rv data_out2(void* ctx, void** ppbuf, size_t* psize, size_t* palloc)
{
    puts("<MORE STDOUT>");
    return CB_OKAY;
}


callback_rv data_err(void* ctx, void** ppbuf, size_t* psize, size_t* palloc)
{
    puts("<STDERR>");
    return CB_OKAY;
}


int main(int argc, char** argv) {
    register_io_callback(0, data_in, 0);
    register_io_callback(1, data_out, 0);
    register_io_callback(1, data_out2, 0);
    register_io_callback(2, data_err, 0);
    pump_execvp(&argv[1]);
}
