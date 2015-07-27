# ldfuck

Corrupts the `link_map` slightly so that remote ELF symbol resolution fails.  In particular, we null out the path of all modules which contain the string `/libc.so` so that the remote end can't find out which module is libc.

## caveats

It's hard to break the link map without breaking lots of things.  This is about as aggressive as we can get :(.  If the attacking team discovers the specific method, they may be able to change their code to perform an index-based lookup.

## example

```
$ cd demo
$ python demo.py ./example
[*] '/home/user/gatekeeper/src/ldfuck/example/example'
    Arch:     amd64-64-little
    RELRO:    Full RELRO
    Stack:    No canary found
    NX:       NX enabled
    PIE:      No PIE
[x] Starting program './example'
[+] Starting program './example': Done
[x] Resolving 'system' in 'libc.so'
[x] Resolving 'system' in 'libc.so': Finding linkmap
[+] Resolving 'system' in 'libc.so': 0x7fa746c681c8
[-] Could not find 'libc.so'
[*] .gnu.hash/.hash, .strtab and .symtab offsets
[*] Found DT_GNU_HASH at 0x7fa746a3cc00
[*] Found DT_STRTAB at 0x7fa746a3cc10
[*] Found DT_SYMTAB at 0x7fa746a3cc20
[*] .gnu.hash parms
[*] hash chain index
[*] hash chain
[*] system: 0x7fa7466c5640
[*] Stopped program './example'
$ LD_PRELOAD=$PWD/../ldfuck LD_BIND_NOT=1 python demo.py example
[*] '/home/user/gatekeeper/src/ldfuck/example/example'
    Arch:     amd64-64-little
    RELRO:    Full RELRO
    Stack:    No canary found
    NX:       NX enabled
    PIE:      No PIE
[x] Starting program './example'
[+] Starting program './example': Done
[x] Resolving 'system' in 'libc.so'
[x] Resolving 'system' in 'libc.so': Finding linkmap
[+] Resolving 'system' in 'libc.so': 0x7f66be4861c8
[-] Could not find 'libc.so'
[*] .gnu.hash/.hash, .strtab and .symtab offsets
[*] Found DT_GNU_HASH at 0x7f66be484e90
[*] Found DT_STRTAB at 0x7f66be484ea0
[*] Found DT_SYMTAB at 0x7f66be484eb0
[*] .gnu.hash parms
[*] hash chain index
[*] hash chain
[-] Could not find a GNU hash that matched 0x1ceee48a
[*] system: 0x0
[*] Stopped program './example'
```
