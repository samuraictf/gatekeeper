# inotify

Kills all processes in its process group when it detects that a file has been opened.

## usage

```sh
$ echo FLAG > flag
$ ./inotify flag sh
$ cat flag
Killed
```