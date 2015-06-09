
# chroot

Starts the target process in a chroot environment.

## notes

The chroot needs to be set up such that all of the dynamic libraries required are available/

## usage

```sh
mkdir -p bin lib lib64 lib/x86_64-linux-gnu
cp {/,}bin/sh
cp {/,}lib/x86_64-linux-gnu/libtinfo.so.5
cp {/,}lib/x86_64-linux-gnu/libdl.so.2
cp {/,}lib/x86_64-linux-gnu/libc.so.6
cp {/,}lib64/ld-linux-x86-64.so.2
./chroot $PWD /bin/sh
```

```sh
sh-4.3$ pwd
/
```