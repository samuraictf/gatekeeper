# onepath

Checks `/proc/self/exe`.  If it's not the expected value (from environment variable `ONE_TRUE_PATH`), the process kills its parent and then itself.

## example

```sh
$ ONE_TRUE_PATH=/bin/bash LD_PRELOAD=$PWD/onepath.so /bin/bash --norc --noprofile
$ echo *
Makefile Makefile.exe onepath.c onepath.o onepath.so README.md
$ /bin/bash -c 'echo Hello'
Hello
$ /bin/dash -c 'echo Hello'
[1]    27247 killed     ONE_TRUE_PATH=/bin/bash LD_PRELOAD=$PWD/onepath.so bash --norc --noprofile
```
