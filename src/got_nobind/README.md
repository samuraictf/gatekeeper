# got_nobind

Does the exact opposite of `LD_BIND_NOW`.

This prevents the GOT resolver from updating entries, so they cannot leak library addresses. Applications may still overwrite the entries directly.

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