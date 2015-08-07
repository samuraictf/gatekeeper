# Gatekeeper

A collection of programs for controlling subprocesses on Linux and related operating systems.

Each module is available as a stand-alone binary, which will either `fork` and `execve`, or apply its mitigation and `execve` directly to the next in the chain.

Each module is also available as a library, which can be compiled into a larger, monolithic binary.

## Requirements

```
apt-get -y install make gcc libpcre3-dev libcap-dev libpcap-dev libseccomp-dev
```

## Building

You can use `clang` or `gcc`.

```
make
make CC=clang-3.5
```

You can also build a specific project.

```
cd src/chroot
make
```

### Cross-Compiling

Just specific the name of the target.  You must have an appropriate compiler toolchain installed.

```
make CROSS_COMPILE=aarch64-linux-gnu
make CROSS_COMPILE=arm-linux-gnueabihf
```

## Testing

Requires [`bats`](https://github.com/sstephenson/bats).

```
make test
```

## Chaining Modules

The modules are designed to be included in a larger project, or chained directly.  For example:

```sh
$ ./blacklist/blacklist \
  ./pcap/pcap foo.pcap \
  ./alarm/alarm 10 \
  ./got_nobind/got_nobind \
  ./malloc/malloc \
  ./no_network/no_network \
  ./randenv/randenv \
  ./rlimit_cpu/rlimit_cpu 5 \
  ./rlimit_fsize/rlimit_fsize 0 \
  ./rlimit_nproc/rlimit_nproc 0 \
  ./segv/segv MYSEGV \
  ./setpgid/setpgid \
  ./setsid/setsid \
  /usr/bin/env LD_PRELOAD="$PWD/ldfuck/ldfuck.so $PWD/no_execve/no_execve.so" \
  /bin/sh
```

## Modules

Here's a short descrption of each module.  To build a module, just run `make` in its directory.

- [`alarm`](src/alarm/README.md) - Kills the process and all of its children with `SIGALRM` after a period of time
- [`blacklist`](src/blacklist/README.md) - Blocks connections from blacklisted IP/IPv6 address ranges, by inspecting `getpeername` on stdin, stdout, stderr.
- [`chroot`](src/chroot/README.md) - Does what it says on the tin.
- [`delay`](src/delay/README.md) - Adds in a time delay after every proxied `read` or `write`
- [`devctf`](src/devctf/README.md) - Hooks calls to `open` to catch `open("/dev/ctf",...)` and returns a pre-determined file descriptor.  This allows access to `/dev/ctf` from within a chroot.
- [`got_bind`](src/got_bind/README.md) - Forces the GOT to bind immediately
- [`got_nobind`](src/got_nobind/README.md) - Prevents the GOT from binding, which prevents ASLR defeats by leaking the GOT
- [`inotify_child`](src/inotify_child/README.md) - Watches only its own child process, and inspects its file table (via `SIGSTOP` and `/proc/.../fd`) to see if any handles are open.
- [`inotify`](src/inotify/README.md) - Watches for events on a specific file, and kills all children (in a new process group) on an event.
- [`ldfuck`](src/ldfuck/README.md) - Fucks with internal linker structures which are used to leak function addresses over-the-wire.
- [`malloc`](src/malloc/README.md) - Sets environment variables understood by glibc and eglibc, which cause `malloc`ed memory to be initialized to a pattern, and `free`ed memory to be overwritten with a pattern.  Useful for heap leaks/UAFs.
- [`no_execve`](src/no_execve/README.md) - Hooks all `exec*` and related (`system`, `popen`) function calls via the PLT.  Also disables `execve` via seccomp-bpf.
- [`no_network`](src/no_network/README.md) - Uses `seccomp` to prevent most socket-related syscalls.
- [`noparent`](src/noparent/README.md) - Performs a double-fork before `execve`ing so that, in the child process, `getppid()` will return `1` (pid of `init`) so that `kill(SIGTERM, getppid())` shellcode is useless.
- [`onepath`](src/onepath/README.md) - Allows `execve` calls, but checks `/proc/self/exe` in the new process to see if it is a specific, permitted path.
- [`openfile`](src/openfile/README.md) - Opens a specific file on a specific file descriptor.
- [`pcap`](src/pcap/README.md) - Captures all stdin/stdout/stderr to a pcap file, with accurate address information gathered from `getpeername`.
- [`proxy`](src/proxy/README.md) - Communications forwarding template and hook library.  Ideally suited to only performing a single copy of stdin/stdout/stderr instead of multiple copies between various consumers.
- [`randenv`](src/randenv/README.md) - Adds a random-length environment variable to the environment, which should modify offsets on the stack.
- [`regex`](src/regex/README.md) - I/O filtering based on regular expressions
- [`rlimit_cpu`](src/rlimit_cpu/README.md) - Adds CPU time limits to all subprocesses
- [`rlimit_fsize`](src/rlimit_fsize/README.md) - Adds limits on the size of file which may be created
- [`rlimit_nproc`](src/rlimit_nproc/README.md) - Effectively prevents child processes from `fork`ing.
- [`segv`](src/segv/README.md) - Installs a `SIGSEGV` handler library, which will dump stack traces to a file.  Requires `libSegFault.so` from `libsegfault`.
- [`setpgid`](src/setpgid/README.md) - Spawns the child in a new process group.  All of its descendants can be easily killed with `kill(SIGTERM, -pid)`.
- [`setsid`](src/setsid/README.md) - Spawns the child in a new session.
- [`signal`](src/signal/README.md) - Masks off all signals.  May interfere with `segv` and `alarm` modules.
- [`unbuffer`](src/unbuffer/README.md) - Effectively disables libc buffering of stdout in the child by creating a pseudo-terminal (PTY) as its stdout (instead of a pipe).
- [`unsocket`](src/unsocket/README.md) - Turns all `send` and `recv` calls, which only work on sockets, into `write` and `read` calls, which work just fine on sockets, files, or pipes.

