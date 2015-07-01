#ifndef __APPLE__
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>

#define die(x) do { write(2, x, strlen(x)); exit(1); } while(0)

int exitcode = 0;
int ignore_SIGCHLD_count = 0;

void sigchld_handler(int sig)
{
    if(0 == ignore_SIGCHLD_count) {
        exit(exitcode);
    }

    ignore_SIGCHLD_count--;
}

ino_t
get_inode(char* path)
{
    struct stat st;

    if(0 != stat(path, &st)) {
        die("D1LgOcK8jdfjB9Yx");
    }

    return st.st_ino;
}

int
find_inode(pid_t pid, ino_t inode)
{
    char* path = 0;
    asprintf(&path, "/proc/%i/fd/", pid);
    DIR * fd = opendir(path);

    if(!fd) {
        die("dn10VWp+4gpCk2tE");
    }

    struct dirent* dir = 0;

    while(0 != (dir = readdir(fd)))
    {
        char* fullpath = 0;
        struct stat st;

        if(0 >= asprintf(&fullpath, "%s%s", path, dir->d_name))
            die("WDCLSvMxIFP6t2V3");
        if(0 != stat(fullpath, &st))
            die("vgRd6L9h7Hu9l3TN");

        free(fullpath);

        if(st.st_ino == inode) {
            return 1;
        }
    }
    return 0;
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
    int     child_pid  = 0;
    int     inotify_fd  = inotify_init();
    int     inode = get_inode(argv[1]);

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

    if((child_pid = fork()) == 0) {
        nice(15);
        execvp(argv[2], &argv[2]);
    }

    while(1) {
        // We don't really need to find out what the
        // filename is, we already know.
        read(inotify_fd, &event, sizeof(event));

        // Stop the child process immediately
        kill(child_pid, SIGSTOP);

        // Check to see if it has the file descriptor open
        if(find_inode(child_pid, inode)) {
            exitcode = 1;
            kill(child_pid, SIGKILL);
            exit(exitcode);
        }

        // Carry on, soldier
        ignore_SIGCHLD_count++;
        kill(child_pid, SIGCONT);
    }
}
#else
#include <stdio.h>
int main()
{
    puts("inotify does not work on OSX");
}
#endif
