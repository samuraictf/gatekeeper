#include <stdlib.h>

__attribute__((constructor))
void
initialize_random
(
)
{
    int seed;
    int urandom = open("/dev/urandom", O_RDONLY);
    read(urandom, &seed, sizeof(seed));
    srand(seed);
    close(urandom);
}