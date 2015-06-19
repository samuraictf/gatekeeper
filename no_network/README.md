# no_network

Disables all socket calls via seccomp

This means that all `recv` and `send` calls must be patched out, or otherwise redirected (e.g. `LD_PRELOAD`) to `read` and `write.`

## usage

```
sh$ ./no_network bash
bash$ whoami
riggle
bash$ cat < /dev/tcp/10.0.0.1/8080
Bad system call
```
