#include <unistd.h>
#include <fcntl.h>



#ifdef __APPLE__
int
pipe2
(
    int fd[2],
    int flags
)
{
    int rc = pipe(fd);
    if(rc >= 0) {
        fcntl(fd[0], F_SETFD, flags);
        fcntl(fd[1], F_SETFD, flags);
    }
    return rc;
}
#endif
