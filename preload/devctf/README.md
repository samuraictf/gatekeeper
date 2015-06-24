# devctf

An LD_PRELOAD library for catching calls to open("/dev/ctf", ...)
so that our chroot still functions properly.

## usage

```sh
$ echo Hello, world > hello
$ exec 1023<>hello
$ LD_PRELOAD=$PWD/devctf.so sh
$ exec 3</dev/ctf
$ ls -la /proc/self/fd
dr-x------ 2 user user  0 Jun 24 00:16 .
dr-xr-xr-x 9 user user  0 Jun 24 00:16 ..
lrwx------ 1 user user 64 Jun 24 00:16 0 -> /dev/pts/98
lrwx------ 1 user user 64 Jun 24 00:16 1 -> /dev/pts/98
lrwx------ 1 user user 64 Jun 24 00:16 1023 -> /home/user/gatekeeper/preload/devctf/hello
lrwx------ 1 user user 64 Jun 24 00:16 2 -> /dev/pts/98
lrwx------ 1 user user 64 Jun 24 00:16 3 -> /home/user/gatekeeper/preload/devctf/hello
lr-x------ 1 user user 64 Jun 24 00:16 4 -> /proc/4825/fd
```
