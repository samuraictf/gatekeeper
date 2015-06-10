# signal

Blocks all blockable signals.

## notes

This is useful to prevent being `SIGTERM`ed.  It cannot stop `SIGKILL` or `SIGSTOP`.

## example

Note that the use of `sleep 1` is just to avoid the scenario where we actually send `SIGTERM` before `sigprocmask()` is invoked.

```sh
$ bash -c 'echo Hello; kill -SIGTERM $$; echo Goodbye'
Hello
[1]    29175 terminated  bash -c 'echo Hello; kill -SIGTERM $$; echo Goodbye'
$ ./signal bash -c 'echo Hello; kill -SIGTERM $$; echo Goodbye' 
Hello
Goodbye
```