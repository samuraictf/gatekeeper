from pwn import *
context.binary = 'example'

p = process(context.binary.path)
p.pack( context.binary.got['read'] )

print "main     %x" % p.unpack()
print "got.read %x" % p.unpack()