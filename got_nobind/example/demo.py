from pwn import *
context.binary = sys.argv[1]

e = context.binary

e = ELF(sys.argv[1])
p = process(e.path)

p.pack(e.got['write'])
a = p.unpack()
b = p.unpack()

log.info(hex(e.got['write']))
log.info(hex(e.plt['write']))
log.info(hex(a))
log.info(hex(b))

# GOT was resolved normally
if a != b:
	sys.exit(0)

# Still points to PLT after function call
if b == e.plt['write']:
	sys.exit(1)

# Originally did not point to PLT
if a != e.plt['write']:
	sys.exit(2)