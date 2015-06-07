#include "fd.h"

int
create_pipes_fork_and_exec
(
    proxied_fd *std_in,
    proxied_fd *std_out,
    char **argv
);