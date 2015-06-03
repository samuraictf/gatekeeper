#include "inotify.h"

static size_t min(size_t a, size_t b) {
    return a < b ? a : b;
}

int main(int argc, char *argv[]) {
    log_fd = 0;
    if (argc !=2) {
        printf("usage: %s <watch file>\n", argv[0]);
        exit(0);
    }
    start_inotify_handler(argv[1], (__sighandler_t) signal_handler);
}

void signal_handler(int signo) {
    printf("signal number: %d", signo);
    return;
}

/* this function will setup an inotify watch on the specified file path
   it will fork and run in a loop watching for inotify events on the file
   a new signal handler will be registered in the parent for SIGUSR1 to
   handle inotify events */
void start_inotify_handler(char * keyfile, __sighandler_t handler) {
    int inotify_fd = -1;
    int watch_fd = -1;
    size_t length = 0;
    pid_t ppid = getpid();
    struct sigaction sig;
    char name[PATH_MAX * 2];

    sigemptyset(&sig.sa_mask);
    sig.sa_flags = 0;
    sig.sa_handler = (__sighandler_t) handler;

    inotify_fd = inotify_init();
    if (inotify_fd < 0) {
        Log("Error inotify_init()\n");
        goto cleanup;
    }

    watch_fd = inotify_add_watch(inotify_fd, keyfile, IN_OPEN | IN_ACCESS);
    if (watch_fd < 0) {
        Log("Error add watch: %s\n", keyfile);
        goto cleanup;
    }

    if (fork() != 0) {
        if (sigaction(SIGUSR1, &sig, NULL) == -1) {
            Log("failure registering sigaction for SIGUSR1") ;
        }

        goto cleanup;
    }

    while (1) {
        struct inotify_event event;

        length = ctf_readn(watch_fd, &event, sizeof(event));

        if ( length < sizeof(event) ) {
            Log("inotify read failure\n");
            goto cleanup;
        }

        if (!event.len) {
            continue;
        }

        event.len = min(event.len, sizeof(name) - 1);
        length = ctf_readn(watch_fd, name, event.len);
        name[sizeof(name) - 1] = 0;

        if (length != event.len) {
            Log("inotify read failure\n");
            goto cleanup;
        }

        if ( event.mask & IN_ACCESS ) {
            Log("File %s was accessed\n", name);
            kill(ppid, SIGUSR1);
        }

        else if ( event.mask & IN_OPEN ) {
            Log("File %s was opened\n", name);
            kill(ppid, SIGUSR1);
        }
    }

cleanup:
    if (watch_fd != -1) {
        inotify_rm_watch(inotify_fd, watch_fd);
    }
    if (inotify_fd != -1) {
        close(inotify_fd);
    }
}
