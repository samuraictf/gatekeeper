#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

void sigchld_handler(int dontcare)
{
    exit(dontcare);
}

int
main
(
    int     argc,
    char    **argv
)
{
    if (argc < 3)
    {
        printf("usage: %s <file> argv0 argv1 ...\n", argv[0]);
        exit(0);
    }

    struct inotify_event event;
    int     pgid     = getpgid(0);
    int     kill_pid = -pgid;
    int     inotify_fd  = inotify_init();

    memset(&event, 0, sizeof(event));

    if(inotify_fd < 0) {
        perror("init");
    }

    int     watch_fd = inotify_add_watch(inotify_fd, argv[1], IN_OPEN);

    if(watch_fd < 0) {
        perror("watch");
    }

    fcntl(inotify_fd, F_SETFD, O_CLOEXEC);
    fcntl(watch_fd, F_SETFD, O_CLOEXEC);

    signal(SIGCHLD, sigchld_handler);

    if(fork() == 0) {
        execvp(argv[2], &argv[2]);
    }

    // We don't really need to find out what the filename is, we already know.
    read(inotify_fd, &event, sizeof(event));

    // Kill all processes in our process group.
    kill(kill_pid, SIGKILL);
}

