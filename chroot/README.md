
# chroot

Starts the target process in a chroot environment.

## notes

The chroot needs to be set up such that all of the dynamic libraries required are available/

## usage

```sh
$ ./chroot example /bin/sh
$ pwd
/
```