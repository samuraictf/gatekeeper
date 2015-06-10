# got_nobind

Does the exact opposite of `LD_BIND_NOW`.  This prevents the GOT entries from ever being updated, so they cannot be used as leaks.

They can still be overwritten.

# example

See the `example/` directory.

### normal execution

```sh
$ python example.py SILENT
main     40057d
got.read 7f9605dd3800
```

### with got_nobind

```sh
$ ../got_nobind python example.py SILENT
main     40057d
got.read 400466
```