# inotify_child

Like `inotify`, but only inspects its direct child to see if it has opened a file.



## usage

Works for slow things.

```sh
$ echo FLAG > flag
$ ./inotify_child flag sh -c 'while read line; do echo $line; done < flag'
```

Doesn't work for things that work like shellcode (immediate `open`, `read`, `write`).

```sh
$ echo FLAG > flag
$ ./inotify_child flag sleepcat/sleepcat flag
FLAG
```
