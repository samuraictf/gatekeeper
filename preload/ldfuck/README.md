# ldfuck

Corrupts the `link_map` slightly so that remote ELF symbol resolution fails.  In particular, we null out the path of all modules which contain the string `/libc.so` so that the remote end can't find out which module is libc.

## caveats

It's hard to break the link map without breaking lots of things.  This is about as aggressive as we can get :(.  If the attacking team discovers the specific method, they may be able to change their code to perform an index-based lookup.

## example

TBD.
