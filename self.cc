#include <stdint.h>

uintptr_t start = 0, stop = 0;

void
find_self
(
)
{
    uint32_t *self = (uint32_t) &find_self;
    self &= ~(0xfff);

    while (*self != '\x7FELF')
    {
        self -= 0x1000;
    }

    // We'll be dump and assume 1MB.
    start   = (void *) self;
    stop    = self + 0x10000;
}
