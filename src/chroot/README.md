
# chroot

Starts the target process in a chroot environment.  This can either be a bare chroot, or most common world-readable resources can be bind-mounted so that we don't have to copy libraries around.

As an additional benefit, we can also disable access to all network interfaces, disable forking, and prevent the spawned process from being able to kill anything outside the chroot.

## usage

```sh
$ ./chroot $(mktemp) /bin/sh
$ pwd
/
$ echo $$
1
$ ps auxf
USER       PID %CPU %MEM    VSZ   RSS TTY      STAT START   TIME COMMAND
user         1  0.0  0.0  29080  2364 pts/81   S    00:18   0:00 sh
user        14  0.0  0.0  26188  1168 pts/81   R+   00:20   0:00 ps auxf
$ touch /foo
touch: cannot touch ‘/foo’: Read-only file system
$ find /tmp
/tmp
$ crontab -e
/var/spool/cron: No such file or directory
```
