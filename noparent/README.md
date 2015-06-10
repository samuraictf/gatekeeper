# noparent

Prevents the process from finding out the PID of its parent.

## notes

This is useful to protect our monitors from an owned child process doing something like:

```c
kill(getppid(), SIGTERM);
```

We do this by double-forking and killing the intermediary process.  This means that `getppid()` in the child will be `1` (the `init` process).  We use a pipe to communicate the child's PID to the `nopareent` process so that we can `wait` for it to exit and close ourself -- ensuring that `SIGCHLD` is delivered up the stream.

## example

```sh
$ echo PID:$$ PPID:$PPID
PID:11193 PPID:7132
$ /bin/sh -c 'echo PID:$$ PPID:$PPID'
PID:26160 PPID:11193
$ ./noparent /bin/sh -c 'echo PID:$$ PPID:$PPID'
PID:26202 PPID:1
```