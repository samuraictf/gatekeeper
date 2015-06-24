# ldfuck

Corrupts the `link_map` slightly so that remote ELF symbol resolution fails.  In particular, we null out the path of all modules which contain the string `/libc.so` so that the remote end can't find out which module is libc.

## caveats

It's hard to break the link map without breaking lots of things.  This is about as aggressive as we can get :(.  If the attacking team discovers the specific method, they may be able to change their code to perform an index-based lookup.

## example

```
$ cd demo
$ python demo.py
[+] Starting program './demo': Done
[+] Resolving 'system' in 'libc.so': 0x7f6a2d6831c8
[*] Trying lookup based on Build ID: 30c94dc66a1fe95180c3d68d2b89e576d5ae213c
[*] .gnu.hash/.hash, .strtab and .symtab offsets
[*] Found DT_GNU_HASH at 0x7f6a2d457c00
[*] Found DT_STRTAB at 0x7f6a2d457c10
[*] Found DT_SYMTAB at 0x7f6a2d457c20
[*] .gnu.hash parms
[*] hash chain index
[*] hash chain
[*] system: 0x7f6a2d0e0640
[*] Stopped program './demo'
$ export LD_BIND_NOW=1
$ export LD_PRELOAD=$PWD/../ldfuck.so
$ python demo.py
```
