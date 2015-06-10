# openfile

Opens a file on a numbered file descriptor, and executes the rest of the command line.

## example

```sh
$ ./openfile /dev/urandom 1234 sh -c 'head -c64 <&1234' | xxd
0000000: eb23 bbf5 bc68 f974 c298 0e8a 1117 b08c  .#...h.t........
0000010: 02ee af2c 7915 c505 63f7 ab5d d2bf 6a3d  ...,y...c..]..j=
0000020: 3aef 93b5 c89c 3766 068f c2f8 bf60 484f  :.....7f.....`HO
0000030: a373 f641 305b 9539 4208 ae2a 7f48 b49f  .s.A0[.9B..*.H..
```