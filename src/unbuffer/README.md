# unbuffer

Effectively disables buffering of stdio on the child process.

## notes

This effect is removed if the input is proxied between `unbuffer` and the target program.

This works because `eglibc` disables buffering on `stdin` and `stdout` if they look like a TTY (console), but enable buffering if it's a pipe.  This jumps through all of the hooks to create a virtual terminal (PTY) to do this fooling.

## example

Normally, processes which detect that `stdin` is a pipe do processing in batch mode.  In the example below, Python does not actually execute the `print` command until `stdin` closes.

```sh
$ (echo "print 'python'"; sleep 1; echo bash>&2; sleep 1;) | python
bash
python
```

Here, it executes the `print` command immediately.

```sh
$ (echo "print 'python'"; sleep 1; echo bash>&2; sleep 1;) | ./unbuffer python
print 'python'
Python 2.7.9 (default, Jan  1 2015, 14:20:57)
[GCC 4.8.2] on linux2
Type "help", "copyright", "credits" or "license" for more information.
>>> python
>>> bash
```
