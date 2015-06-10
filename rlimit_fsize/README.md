
# rlimit_cpu

Limits the maximum size of a file which may be written.

## usage

```sh
$ ./rlimit_fsize 3 sh
$ echo >> file
$ echo >> file
$ echo >> file
$ echo >> file
[1]    19170 file size limit exceeded (core dumped)  ./rlimit_fsize 3 sh
```
