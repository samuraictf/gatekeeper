from pwn import *

for interface, addresses in net.interfaces().items():
    for family, address in addresses:
        l = listen(fam=family)
        l.spawn_process(['./blacklist', 'echo', 'Howdy, stranger!'])
        try:
            r = remote(address, l.lport, timeout=0.5)
        except:
            continue
        print address, r.recvall()
