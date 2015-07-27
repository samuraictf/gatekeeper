from pwn import *
context.binary = sys.argv[1]


def doit(p):
    def leak(address):
        p.pack(address)
        return p.recvn(context.bytes)

    addr = p.unpack()
    d = DynELF(leak, addr)
    address = (d.lookup('system', 'libc.so') or 0)
    log.info("system: %#x" % address)

    if not address:
    	sys.exit(1)

doit(process(sys.argv[1], stderr=2))
