#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <fcntl.h>              /* Obtain O_* constant definitions */
#include <unistd.h>

#include "exec.h"

//------------------------- CREATE PIPES FORK AND EXEC -------------------------
/**
 * Create all of the pipes necessary for communication with the child process,
 * spawn it, and copy the pipes into the source/sink for the local and remote
 * sides.
 *
 * Returns the child pid.
 */
int
create_pipes_fork_and_exec
(
    char **argv
)
{
    //
    // Set up pipes for child
    //
    int pipes[2];
    int std_in_read, std_in_write;
    int std_out_read, std_out_write;

    pipe2(pipes, O_CLOEXEC);
    std_in_read     = pipes[0];
    std_in_write    = pipes[1];

    pipe2(pipes, O_CLOEXEC);
    std_out_read    = pipes[0];
    std_out_write   = pipes[1];

    //
    // Fork off the child
    //
    int child_pid = fork();

    if (child_pid == 0)
    {
        dup2(std_in_read, 0);
        dup2(std_out_write, 1);
        dup2(std_out_write, 2);
        execvp(argv[0], &argv[0]);
        _exit(1);
    }

    //
    // Close unused file descriptors
    //
    close(std_out_write);
    close(std_in_read);

    std_in.source   = STDIN_FILENO;
    std_in.sink     = std_in_write;

    std_out.source  = std_out_read;
    std_out.sink    = STDOUT_FILENO;

    return child_pid;
}
