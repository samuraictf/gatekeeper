# got_nobind example

This is just a trivial example which reads a pointer out of the GOT, which is a common info leak mechanism.

## normal execution

```sh
$ python example.py SILENT
main     40057d
got.read 7f9605dd3800
```

## with got_nobind

```sh
$ ../got_nobind python example.py SILENT
main     40057d
got.read 400466
```