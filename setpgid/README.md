# setpgid

Sets the Process Group ID

## notes

Process groups are useful for culling all child processes.  In particular, you can kill all processes in a process group with a single syscall (`kill(-getpgrp())`)

## usage

Generally you'll want to create a new process group, but it's also useful for attaching to existing ones.

Use `0` to create a new proces group id.`

```
$ ./setpgid 0 sh
$ ps xao comm,pid,ppid,pgid,sid | grep $$
sh              40280 32530 40280 32530
ps              40287 40280 40287 32530
grep            40288 40280 40287 32530
```