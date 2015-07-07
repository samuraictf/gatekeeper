# no_execve

Prevents the process from `execve`ing to a different process, such as `bash`.

## function hooks

Hooks all `exec*` functions, as well as `system` and `popen` and the not-yet-available `execveat`.

## seccomp-bpf

Disallows the execution of `execve` and `execveat` system calls.
Also disallows all system calls we don't know exist (e.g. if new ones are added).
Also disallows all wrong-architecture syscalls (e.g. compatibility mode).

## example

```sh
$ LD_PRELOAD=$PWD/no_execve.so sh
$ ps
$ echo $?
159
```
