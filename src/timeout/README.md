
# alarm

Similar to the `alarm` module, but uses `SIGKILL` instead of `SIGALRM`, which is not blockable.

## usage

```sh
$ ./timeout 1 /bin/sh -c "echo hello; sleep 2; echo nope"
hello
<exited>
```
