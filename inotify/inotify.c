/**
 * This tool monitors a file to see if it's been read or written.
 * Once it has been notified of this, it immediately kills all
 * child processes by killing all processes in the process group.
 *
 * In order to make this a non-disruptive killing process, the tool
 * creates its own process group when it is initialized.
 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/inotify.h>
#include <linux/limits.h>
#include <linux/sysctl.h>
#include <sys/syscall.h>
#include <unistd.h>
       #include <sys/types.h>
       #include <sys/stat.h>
       #include <fcntl.h>

int     child_session;
pid_t   pid_max;
int     tty;

#define log(...) dprintf(tty, __VA_ARGS__)

struct inotify_event event;
char    name[1024];

//******************************************************************************
//                                  STRUCTURES
//******************************************************************************


// struct __sysctl_args
// {
//     int     *name;      /* integer vector describing variable */
//     int     nlen;       /* length of this vector */
//     void    *oldval;    /* 0 or address where to store old value */
//     size_t  *oldlenp;   /* available room for old value,
//                            overwritten by actual size of old value */
//     void    *newval;    /* 0 or address of new value */
//     size_t  newlen;     /* size of new value */
// };


//******************************************************************************
//                                  PROTOTYPES
//******************************************************************************
void
handle_SIGCHLD
(
    int
);


int
fork_setsid_and_exec
(
    char **argv
);


size_t
get_kernel_pidmax
(
);


void
stop_all_processes_in_session
(
);


void
kill_all_processes_in_session
(
);


void
signal_all_processes_in_session
(
    int session_id,
    int signal
);


char *
inotify_loop
(
    char    * file,
    void (  *callback)()
);


//******************************************************************************
//                               IMPLEMENTATIONS
//******************************************************************************

static size_t
min
(
    size_t  a,
    size_t  b
)
{
    return a < b ? a : b;
}



void
handle_SIGCHLD
(
    int nofucksgiven
)
{
    // The child has exited, we don't really care why.
    // Kill everything else in the session.
    kill_all_processes_in_session();
    _exit(0);
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

    signal(SIGCHLD, handle_SIGCHLD);

    pid_max         = get_kernel_pidmax();
    child_session   = fork_setsid_and_exec(&argv[2]);
    tty             = open("/dev/tty", O_RDWR);

    inotify_loop(argv[1], stop_all_processes_in_session);
}



/**
 * Forks, creatse a new session, and spawns a process inside of it
 * with the provided arguments
 *
 * @param  argv Arguments to the child
 *
 * @return Child PID (same as the sesion ID)
 */
int
fork_setsid_and_exec
(
    char **argv
)
{
    int child_pid = fork();

    if (child_pid == 0)
    {
        setsid();
        execvp(argv[0], &argv[0]);
        _exit(1);
    }

    return child_pid;
}



/**
 * @return Maximum PID supported on the running system.
 */
size_t
get_kernel_pidmax
(
)
{
    struct __sysctl_args args;
    size_t  pidmax      = 0;
    size_t  pidmax_len  = sizeof(pidmax);

    int     name[]      = {CTL_KERN, KERN_PIDMAX};
    int     namelen     = 2;

    args.name       = name;
    args.nlen       = sizeof(name) / sizeof(name[0]);
    args.oldval     = &pidmax;
    args.oldlenp    = &pidmax_len;

    if (-1 == syscall(SYS__sysctl, &args))
    {
        pidmax = 65535 * 2;
    }

    return pidmax;
}



/**
 * Stops all processes in the child's session via SIGSTOP.
 * (which, like SIGKILL, is unblockable).
 */
void
stop_all_processes_in_session
(
)
{
    signal_all_processes_in_session(child_session, SIGSTOP);
}



/**
 * Kills all processes in the current session
 */
void
kill_all_processes_in_session
(
)
{
    signal_all_processes_in_session(child_session, SIGKILL);
}



/**
 * Signals all of the processes in the specified session
 */
void
signal_all_processes_in_session
(
    int session_id,
    int signal
)
{
    for (pid_t pid = 0; pid < pid_max; pid++)
    {
        if (getsid(pid) != session_id)
        {
            continue;
        }

        kill(pid, signal);
    }
}



/**
 * Reads from a file descriptor until given length is reached.
 * Returns number of bytes received.
 *
 * @param  fd   File descriptor to read from
 * @param  msg_ Buffer to read into
 * @param  len  Number of bytes to read
 *
 * @return Number of bytes read successfully
 */
int
readn
(
    const int   fd,
    void        *msg_,
    size_t      len
)
{
    ssize_t prev    = 0;
    size_t  count   = 0;
    char    * msg   = (char *) msg_;

    if ((fd >= 0) && msg && len)
    {
        for (count = 0; count < len; count += prev)
        {
            prev = read(fd, msg + count, len - count);

            if (prev <= 0)
            {
                break;
            }
        }
    }

    return count;
}



/**
 * Sets up inotify and waits until something happens.
 *
 * @param  file
 * Path to the file to monitor.
 *
 * @param  callback
 * Function to invoke immediately after receiving data, before
 * parsing anything out of the structure.
 *
 * @return      [description]
 */

char *
inotify_loop
(
    char    * file,
    void (  *callback)()
)
{
    int     watch_fd    = -1;
    size_t  length      = 0;
    pid_t   ppid        = getpid();
    struct sigaction sig;
    char    name[PATH_MAX * 2];

    int     inotify_fd  = inotify_init();

    if (inotify_fd < 0)
    {
        Log("Error inotify_init()\n");
        goto cleanup;
    }

    watch_fd = inotify_add_watch(inotify_fd, keyfile, IN_OPEN | IN_ACCESS);

    if (watch_fd < 0)
    {
        Log("Error add watch: %s\n", keyfile);
        goto cleanup;
    }

    while (1)
    {
        length = readn(watch_fd, &event, sizeof(event));

        // Kill now!
        stop_all_processes_in_session();

        if (length < sizeof(event))
        {
            Log("inotify read failure\n");
            goto cleanup;
        }

        if (!event.len)
        {
            continue;
        }

        event.len               = min(event.len, sizeof(name) - 1);
        length                  = ctf_readn(watch_fd, name, event.len);
        name[sizeof(name) - 1]  = 0;
        break;

        if (length == event.len)
        {
            return strdup(name);
        }
    }

cleanup:

    if (watch_fd != -1)
    {
        inotify_rm_watch(inotify_fd, watch_fd);
    }

    if (inotify_fd != -1)
    {
        close(inotify_fd);
    }
}
