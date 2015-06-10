
An LD_PRELOAD library for catching calls to open("/dev/ctf", ...)
so that our chroot still functions properly.

In order to build for an alternate architecture, just do something like:

    $ make clean all CC=arm-linux-androideabi-gcc CXX=arm-linux-androideabi-g++

Usage is straightforward:

    $ echo Hello, world > hello
    $ exec 1023<>hello
    $ LD_PRELOAD=$PWD/devctf.so python <<EOF
print open("/dev/ctf").read()
EOF