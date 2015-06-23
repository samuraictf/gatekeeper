# blacklist

Prevents connections from IP address ranges.

## usage

The demo app prevents connections from localhost over ipv4 and ipv6.

```
$ python test.py SILENT
127.0.0.1 No connections from localhost

::1 No connections from localhost

192.168.122.1 Howdy, stranger!

172.31.247.135 Howdy, stranger!

[ERROR] Could not connect to fe80::42a8:e0ff:fe68:d9a3 on port 58287
192.168.99.1 Howdy, stranger!

[ERROR] Could not connect to fe80::250:46ff:fec0:8 on port 38483
172.16.16.1 Howdy, stranger!

[ERROR] Could not connect to fe80::250:76ff:fec0:1 on port 34708
```
