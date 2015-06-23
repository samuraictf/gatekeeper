# Gatekeeper

A collection of programs for controlling subprocesses on Linux and related operating systems.

Each module is available as a stand-alone binary, which will either `fork` and `execve`, or apply its mitigation and `execve` directly to the next in the chain.

Each module is also available as a library, which can be compiled into a larger, monolithic binary.

## Modules

Here's a short descrption of each module.  To build a module, just run `make` in its directory.

- `alarm` - Kills the process and all of its children with `SIGALRM` after a period of time
- `blacklist` - Blocks connections from blacklisted IP/IPv6 address ranges, by inspecting `getpeername` on stdin, stdout, stderr.
- `chroot` - Does what it says on the tin.
- `delay` - Adds in a time delay after every proxied `read` or `write`
- `got_bind` - Forces the GOT to bind immediately
- `got_nobind` - Prevents the GOT from binding, which prevents ASLR defeats by leaking the GOT
- `inotify` - Watches for events on a specific file, and kills all children (in a new process group) on an event.
- `inotify_child` - Watches only its own child process, and inspects its file table (via `SIGSTOP` and `/proc/.../fd`) to see if any handles are open.
- `malloc` - Sets environment variables understood by glibc and eglibc, which cause `malloc`ed memory to be initialized to a pattern, and `free`ed memory to be overwritten with a pattern.  Useful for heap leaks/UAFs.
- `no_network` - Uses `seccomp` to prevent most socket-related syscalls.
- `noparent` - Performs a double-fork before `execve`ing so that, in the child process, `getppid()` will return `1` (pid of `init`) so that `kill(SIGTERM, getppid())` shellcode is useless.
- `openfile` - Opens a specific file on a specific file descriptor.
- `pcap` - Captures all stdin/stdout/stderr to a pcap file, with accurate address information gathered from `getpeername`.
- `preload` - A collection of `LD_PRELOAD` modules.
- `proxy` - Communications forwarding template and hook library.  Ideally suited to only performing a single copy of stdin/stdout/stderr instead of multiple copies between various consumers.
- `randenv` - Adds a random-length environment variable to the environment, which should modify offsets on the stack.
- `rlimit_cpu` - Adds CPU time limits to all subprocesses
- `rlimit_fsize` - Adds limits on the size of file which may be created
- `rlimit_nproc` - Effectively prevents child processes from `fork`ing.
- `segv` - Installs a `SIGSEGV` handler library, which will dump stack traces to a file.  Requires `libSegFault.so` from `libsegfault`.
- `setpgid` - Spawns the child in a new process group.  All of its descendants can be easily killed with `kill(SIGTERM, -pid)`.
- `setsid` - Spawns the child in a new session.
- `signal` - Masks off all signals.  May interfere with `segv` and `alarm` modules.
- `unbuffer` - Effectively disables libc buffering of stdout in the child by creating a pseudo-terminal (PTY) as its stdout (instead of a pipe).

## Future Modules

These modules do not work yet.

- `regex` - I/O filtering based on regular expressions
- `stderr` - Logs all output of `stderr` to a file.
- `breakpad` - Loads breakpad (crash reporting) into the child process.
