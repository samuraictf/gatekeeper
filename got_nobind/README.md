# got_nobind

Does the exact opposite of `LD_BIND_NOW`.  This prevents the GOT entries from ever being updated, so they cannot be used as leaks.

They can still be overwritten.