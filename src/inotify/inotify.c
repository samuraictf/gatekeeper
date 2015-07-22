#ifndef __APPLE__
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

void sigchld_handler(int dontcare)
{
    exit(dontcare);
}

char* print_mask(int mask) {
#define CASE_RETURN(x) case x: return #x;
    switch(mask) {
        CASE_RETURN(IN_ACCESS)
        CASE_RETURN(IN_ATTRIB)
        CASE_RETURN(IN_CLOSE_WRITE)
        CASE_RETURN(IN_CLOSE_NOWRITE)
        CASE_RETURN(IN_CREATE)
        CASE_RETURN(IN_DELETE)
        CASE_RETURN(IN_DELETE_SELF)
        CASE_RETURN(IN_MODIFY)
        CASE_RETURN(IN_MOVE_SELF)
        CASE_RETURN(IN_MOVED_FROM)
        CASE_RETURN(IN_MOVED_TO)
        CASE_RETURN(IN_OPEN)
    }
    return "UNKNOWN";
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
    union {
    struct inotify_event event;
    char   buffer[1024];
    }  u;
    int     pgid     = getpgid(0);
    int     kill_pid = -pgid;
    int     inotify_fd  = inotify_init();

    memset(&u.buffer, 0, sizeof(u.buffer));

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
    read(inotify_fd, &u.buffer, sizeof(u.buffer));

#if DEBUG
    dprintf(2, "inotify %i %s %i \"%s\"\n", u.event.wd, print_mask(u.event.mask), u.event.len, u.event.name);
#endif

    // Kill all processes in our process group.
    kill(kill_pid, SIGKILL);
}
#else
#include <stdio.h>
int main()
{
    puts("inotify does not work on OSX");
}
#endif
