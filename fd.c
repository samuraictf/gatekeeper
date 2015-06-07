#define _GNU_SOURCE
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

/*
 * Sets all file descriptors to close on execve.
 *
 * This means that even if they manage to spawn a shell or other command,
 * they won't be able to interact with it, since stdin/stdout/stderr won't
 * exist.
 */
void
set_cloexec_on_all_fds
(
)
{
    for (int i = 0; i < 3; i++)
    {
        fcntl(i, F_SETFD, FD_CLOEXEC);
    }
}


/*
 * Randomizes the selected file descriptor to a new value.
 */
int
randfd
(
    int fd
)
{
    int newfd;

    do
    {
        newfd = rand() % 2048;
    }
    while (-1 == fcntl(newfd, F_GETFD) && errno == EBADF);

    return newfd;
}
