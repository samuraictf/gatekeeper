/* unbuffer.c

   Use a pseudoterminal to circumvent the block buffering performed by the
   stdio library when standard output is redirected to a file or pipe.
   When a program is started using 'unbuffer', its output is redirected to
   a pseudoterminal, and is consequently line-buffered.

   Usage: unbuffer prog [arg ...]
 */
#define _XOPEN_SOURCE /* See feature_test_macros(7) */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <string.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <limits.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

void
errExit
(
    char * msg
)
{
    perror(msg);
    exit(1);
}


/* Open a pty master, returning file descriptor, or -1 on error.
   On successful completion, the name of the corresponding pty
   slave is returned in 'slaveName'. 'snLen' should be set to
   indicate the size of the buffer pointed to by 'slaveName'. */

int
ptyMasterOpen
(
    char    *slaveName,
    size_t  snLen
)
{
    int masterFd, savedErrno;
    char *p;

    masterFd = open("/dev/ptmx", O_RDWR | O_NOCTTY);         /* Open pty master */

    if (masterFd == -1)
    {
        return -1;
    }

    if (grantpt(masterFd) == -1)                /* Grant access to slave pty */
    {
        savedErrno  = errno;
        close(masterFd);                        /* Might change 'errno' */
        errno       = savedErrno;
        return -1;
    }

    if (unlockpt(masterFd) == -1)               /* Unlock slave pty */
    {
        savedErrno  = errno;
        close(masterFd);                        /* Might change 'errno' */
        errno       = savedErrno;
        return -1;
    }

    p = (char *) ptsname(masterFd);                      /* Get slave pty name */

    if (p == NULL)
    {
        savedErrno  = errno;
        close(masterFd);                        /* Might change 'errno' */
        errno       = savedErrno;
        return -1;
    }

    if (strlen(p) < snLen)
    {
        strncpy(slaveName, p, snLen);
    }
    else                      /* Return an error if buffer too small */
    {
        close(masterFd);
        errno = EOVERFLOW;
        return -1;
    }

    return masterFd;
}



struct termios ttyOrig;
static void
/* Reset terminal mode on program exit */
ttyReset
(
    void
)
{
    if (tcsetattr(STDIN_FILENO, TCSANOW, &ttyOrig) == -1)
    {
        // errExit("tcsetattr");
    }
}



pid_t
ptyFork
(
    int                     *masterFd,
    char                    *slaveName,
    size_t                  snLen,
    const struct termios    *slaveTermios,
    const struct winsize    *slaveWS
)
{
    int     mfd, slaveFd, savedErrno;
    pid_t   childPid;
    char    slname[PATH_MAX+1];

    mfd = ptyMasterOpen(slname, PATH_MAX);

    if (mfd == -1)
    {
        return -1;
    }

    if (slaveName != NULL)              /* Return slave name to caller */
    {
        if (strlen(slname) < snLen)
        {
            strncpy(slaveName, slname, snLen);

        }
        else                          /* 'slaveName' was too small */
        {
            close(mfd);
            errno = EOVERFLOW;
            return -1;
        }
    }

    childPid = fork();

    if (childPid == -1)                 /* fork() failed */
    {
        savedErrno  = errno;            /* close() might change 'errno' */
        close(mfd);                     /* Don't leak file descriptors */
        errno       = savedErrno;
        return -1;
    }

    if (childPid != 0)                  /* Parent */
    {
        *masterFd = mfd;                /* Only parent gets master fd */
        return childPid;                /* Like parent of fork() */
    }

    /* Child falls through to here */

    if (setsid() == -1)                 /* Start a new session */
    {
        errExit("ptyFork:setsid");
    }

    close(mfd);                         /* Not needed in child */

    slaveFd = open(slname, O_RDWR);     /* Becomes controlling tty */

    if (slaveFd == -1)
    {
        errExit("ptyFork:open-slave");
    }

 #ifdef TIOCSCTTY                        /* Acquire controlling tty on BSD */

    if (ioctl(slaveFd, TIOCSCTTY, 0) == -1)
    {
        errExit("ptyFork:ioctl-TIOCSCTTY");
    }

 #endif

    if (slaveTermios != NULL)           /* Set slave tty attributes */
    {
        if (tcsetattr(slaveFd, TCSANOW, slaveTermios) == -1)
        {
            errExit("ptyFork:tcsetattr");
        }
    }

    if (slaveWS != NULL)                /* Set slave tty window size */
    {
        if (ioctl(slaveFd, TIOCSWINSZ, slaveWS) == -1)
        {
            errExit("ptyFork:ioctl-TIOCSWINSZ");
        }
    }

    /* Duplicate pty slave to be child's stdin, stdout, and stderr */

    if (dup2(slaveFd, STDIN_FILENO) != STDIN_FILENO)
    {
        errExit("ptyFork:dup2-STDIN_FILENO");
    }

    if (dup2(slaveFd, STDOUT_FILENO) != STDOUT_FILENO)
    {
        errExit("ptyFork:dup2-STDOUT_FILENO");
    }

    if (dup2(slaveFd, STDERR_FILENO) != STDERR_FILENO)
    {
        errExit("ptyFork:dup2-STDERR_FILENO");
    }

    if (slaveFd > STDERR_FILENO)        /* Safety check */
    {
        close(slaveFd);                 /* No longer need this fd */

    }

    return 0;                           /* Like child of fork() */
}



/* Place terminal referred to by 'fd' in cbreak mode (noncanonical mode
   with echoing turned off). This function assumes that the terminal is
   currently in cooked mode (i.e., we shouldn't call it if the terminal
   is currently in raw mode, since it does not undo all of the changes
   made by the ttySetRaw() function below). Return 0 on success, or -1
   on error. If 'prevTermios' is non-NULL, then use the buffer to which
   it points to return the previous terminal settings. */

int
ttySetCbreak
(
    int             fd,
    struct termios  *prevTermios
)
{
    struct termios t;

    if (tcgetattr(fd, &t) == -1)
    {
        return -1;
    }

    if (prevTermios != NULL)
    {
        *prevTermios = t;
    }

    t.c_lflag       &= ~(ICANON | ECHO);
    t.c_lflag       |= ISIG;

    t.c_iflag       &= ~ICRNL;

    t.c_cc[VMIN]    = 1;                    /* Character-at-a-time input */
    t.c_cc[VTIME]   = 0;                    /* with blocking */

    if (tcsetattr(fd, TCSAFLUSH, &t) == -1)
    {
        return -1;
    }

    return 0;
}



/* Place terminal referred to by 'fd' in raw mode (noncanonical mode
   with all input and output processing disabled). Return 0 on success,
   or -1 on error. If 'prevTermios' is non-NULL, then use the buffer to
   which it points to return the previous terminal settings. */

int
ttySetRaw
(
    int             fd,
    struct termios  *prevTermios
)
{
    struct termios t;

    if (tcgetattr(fd, &t) == -1)
    {
        return -1;
    }

    if (prevTermios != NULL)
    {
        *prevTermios = t;
    }

    t.c_lflag       &= ~(ICANON | ISIG | IEXTEN | ECHO);
    /* Noncanonical mode, disable signals, extended
       input processing, and echoing */

    t.c_iflag       &= ~(BRKINT | ICRNL | IGNBRK | IGNCR | INLCR
                         | INPCK | ISTRIP | IXON | PARMRK);
    /* Disable special handling of CR, NL, and BREAK.
       No 8th-bit stripping or parity error handling.
       Disable START/STOP output flow control. */

    t.c_oflag       &= ~OPOST;              /* Disable all output processing */

    t.c_cc[VMIN]    = 1;                    /* Character-at-a-time input */
    t.c_cc[VTIME]   = 0;                    /* with blocking */

    if (tcsetattr(fd, TCSAFLUSH, &t) == -1)
    {
        return -1;
    }

    return 0;
}



int
main
(
    int     argc,
    char    *argv[]
)
{
    char    slaveName[PATH_MAX+1];
    int     masterFd;
    fd_set  inFds;
    static char buf[4096];
    ssize_t numRead;
    struct winsize ws;

    /* Retrieve the attributes of terminal on which we are started */

    if (tcgetattr(STDIN_FILENO, &ttyOrig) == -1)
    {
        // errExit("tcgetattr");
    }

    if (ioctl(STDIN_FILENO, TIOCGWINSZ, (char *) &ws) < 0)
    {
        // errExit("TIOCGWINSZ error");
    }

    /* Create a child process, with parent and child connected via a
       pty pair. The child is connected to the pty slave and its terminal
       attributes are set to be the same as those retrieved above. */

    if (0 == ptyFork(&masterFd, slaveName, PATH_MAX, &ttyOrig, &ws))
    {
        execvp(argv[1], &argv[1]);
        errExit("execvp");
    }

    /* Place terminal in raw mode so that we can pass all
       terminal input to the pseudoterminal master untouched */

    ttySetRaw(STDIN_FILENO, &ttyOrig);

    if (atexit(ttyReset) != 0)
    {
        errExit("atexit");
    }

    /* Loop monitoring terminal and pty master for input. If the
       terminal is ready for input, read some bytes and write
       them to the pty master. If the pty master is ready for
       input, read some bytes and write them to the terminal. */

    while(1)
    {
        FD_ZERO(&inFds);
        FD_SET(STDIN_FILENO, &inFds);
        FD_SET(masterFd, &inFds);

        if (select(masterFd + 1, &inFds, NULL, NULL, NULL) == -1)
        {
            errExit("select");
        }

        if (FD_ISSET(STDIN_FILENO, &inFds))
        {
            numRead = read(STDIN_FILENO, buf, sizeof(buf));

            if (numRead <= 0)
            {
                exit(EXIT_SUCCESS);
            }

            if (write(masterFd, buf, numRead) != numRead)
            {
                errExit("partial/failed write (masterFd)");
            }
        }

        if (FD_ISSET(masterFd, &inFds))
        {
            numRead = read(masterFd, buf, sizeof(buf));

            if (numRead <= 0)
            {
                exit(EXIT_SUCCESS);
            }

            if (write(STDOUT_FILENO, buf, numRead) != numRead)
            {
                errExit("partial/failed write (STDOUT_FILENO)");
            }
        }
    }
}
