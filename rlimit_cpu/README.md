
# rlimit_cpu

Limits the maximum amount of CPU time that a process can use.

## notes

This counter is per-PID, so doing a `fork` resets the counter.

## usage

```sh
$ ./rlimit_cpu 1 sh
$ while true; do true; done
[1]    9442 killed     ./rlimit_cpu 1 sh
```
