# Cross Compilation

Cross-compilation is relatively straightforward once you have the cross-compilation toolchains installed.

## Installing Cross-Compilers

On Ubuntu and Debian, you should be able to do the following to get most of the architectures that we care about.

```sh
apt-get install libc6:i386
apt-get install libc6-dbg:i386
apt-get install linux-libc-dev:i386
apt-get install gcc-aarch64-linux-gnu
apt-get install g++-aarch64-linux-gnu
apt-get install gcc-arm-linux-gnueabihf
apt-get install g++-arm-linux-gnueabihf
apt-get install gcc-powerpc-linux-gnu
apt-get install g++-powerpc-linux-gnu
```

If you also need to build MIPS, SPARC, or S390, you can get the cross-compiler from Emdebian.  The following works on either Ubuntu or Debian.

```sh
apt-get install debian-keyring emdebian-archive-keyring debian-archive-keyring

sudo tee /etc/apt/sources.list.d/emdebian.list << EOF
deb http://mirrors.mit.edu/debian squeeze main
deb http://www.emdebian.org/debian squeeze main
EOF

sudo apt-get update

apt-get install --force-yes gcc-4.4-mips-linux-gnu
apt-get install --force-yes g++-4.4-mips-linux-gnu
apt-get install --force-yes gcc-4.4-s390-linux-gnu
apt-get install --force-yes g++-4.4-s390-linux-gnu
apt-get install --force-yes gcc-4.4-sparc-linux-gnu
apt-get install --force-yes g++-4.4-sparc-linux-gnu

# !! IMPORTANT !! Remove the package source.
sudo rm -rf /etc/apt/sources.list.d/emdebian.list*
sudo apt-get update
```

## Cross-Compiling One-off Programs

Building a single app from source is pretty straightforward.  The compiler names will be:

- `arm-linux-gnueabihf-gcc`
- `aarch64-linux-gnu-gcc`
- `mips-linux-gnu-gcc`
- `powerpc-linux-gnu-gcc`

If building manually, you should probably also set the endianness via `-EL` or `-EB`.

## Cross-Compiling Gatekeeper

Just set the prefix in the `CROSS_COMPILE` variable.

```sh
make CROSS_COMPILE=arm-linux-gnueabihf clean all
```

## This Didn't Work!

Zach has hardware for ARM, AArch64, and MIPS.  Alternately, it may help to try a QEMU system image.  There are lots here: https://people.debian.org/~aurel32/qemu/

Zach also has all of these on a flash drive already.
