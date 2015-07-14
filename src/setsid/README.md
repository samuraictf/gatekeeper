# setpgid

Sets the Process Group ID

## notes

Process groups are useful for culling all child processes.  In particular, you can kill all processes in a process group with a single syscall (`kill(-getpgrp())`)

## usage

Generally you'll want to create a new process group, but it's also useful for attaching to existing ones.

Use `0` to create a new proces group id.`

```
$ ./setpgid 0 sh
$ ps  xao pid,ppid,pgid,sid | grep $$
 9647 18575  9647 18575
 9648  9647  9648 18575
 9649  9647  9648 18575
```