
# rlimit_nproc

Limits the maximum number of processes which can be active as the current user before a `fork` request is permitted.

## notes

The count is **not** the number of children that this process may spawn.  It is the total limit/number of processes running as the current user, which is checked before this process (or any descendants) may `fork`.

As such, it's safe to set this to `0` as long as no `fork`s occur.  This limit is not checked on `execve`, as no new PIDs are used.

## usage

In this example, you can see that things which do not result in a `fork` work just fine.  This includes shell builtins (globbing, echo, read) as well as `exec`ing a new process.  However, forking (as is done when executing a binary like `head`) does not work.

```sh
$ ./rlimit_nproc 0 sh
$ echo *
Makefile README.md rlimit_nproc rlimit_nproc.c rlimit_nproc.o
$ read line < rlimit_nproc.c
$ echo $line
#define _GNU_SOURCE
$ head -n1 rlimit_nproc.c
sh: fork: retry: No child processes
$ exec head -n1 rlimit_nproc.c
#define _GNU_SOURCE
```
