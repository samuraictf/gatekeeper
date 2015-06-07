#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <fcntl.h>              /* Obtain O_* constant definitions */
#include <unistd.h>

typedef void (*pump_callback)(int source, int sink);

struct pump {
    pump_callback callback;

    // Where does data come from? Source.
    // - true stdin
    // - child stdout
    int source;

    union {
        int pipes[2];
        struct {
            // Where do we send data to, when it comes from
            // source, after filtering?  Child's stdin, real stdout.
            int sink;
            int next;
        };
    };
};

enum direction {
    IN, OUT
};

void initialize_pump(direction d, struct pump* p, int fd) {
    pipe(p->pipes);

    // Randomize the file descriptors from the pipe()
    p->source   = randfd(p->source);
    p->sink     = randfd(p->sink);

    // Create a duplicate, random file descriptor for the original input
    p->original = randfd(dup(fd));
}