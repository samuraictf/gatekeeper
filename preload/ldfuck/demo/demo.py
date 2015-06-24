from pwn import *
context.arch = 'amd64'
context.bytes = 8


def doit(p):
    def leak(address):
        p.pack(address)
        return p.recvn(8)

    addr = p.unpack()
    d = DynELF(leak, addr)
    log.info("system: %#x" % (d.lookup('system', 'libc.so') or 0))

doit(process("./demo", stderr=2))
print '#########################################################'
os.environ['LD_PRELOAD'] = os.path.join('.','..','ldfuck.so')
doit(process("./demo", stderr=2))
