# openfile

Opens a file on a numbered file descriptor, and executes the rest of the command line.

## example

    $ ./openfile /dev/zero 1234 /bin/sh
    $ head -c 32 /proc/$$/fd/1234 | xxd
    0000000: 0000 0000 0000 0000 0000 0000 0000 0000  ................
    0000010: 0000 0000 0000 0000 0000 0000 0000 0000  ................
