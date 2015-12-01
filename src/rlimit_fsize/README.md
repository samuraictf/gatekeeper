
# rlimit_fsize

Limits the maximum size of a file which may be written.

## usage

```sh
$ ./rlimit_fsize 3 sh
$ echo -n 'A' >> file1
$ echo -n 'AA' >> file2
$ echo -n 'AAA' >> file3
$ echo -n 'AAAA' >> file4
[1]    19170 file size limit exceeded (core dumped)  ./rlimit_fsize 3 sh
```
