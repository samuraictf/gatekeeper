# pcap

Captures all data from `stdin` and `stdout` into a PCAP file in `/tmp`.

## notes

Writes the name of the `pcap` file to `/dev/tty` so that it's more useful when doing testing and development.  This does not show up when invoked via `xinetd`.

In order to make it easy to scrape the `.pcap` files which are completed (i.e. no new data), in-progress files are prefixed with a `.` (dot).

This tool will capture the real, actual IP and port information if `stdin` is the original socket, and not a pipe.

## usage

```sh
$ ./pcap foo.pcap whoami
riggle
$ tcpdump -XXr foo.pcap
reading from file foo.pcap, link-type EN10MB (Ethernet)
18:20:40.976446 IP localhost.5678 > localhost.1234: [|tcp]
        0x0000:  0000 0000 0000 0000 0000 0000 0800 4500  ..............E.
        0x0010:  001b 0000 0000 0006 0000 7f00 0001 7f00  ................
        0x0020:  0001 162e 04d2 0000 0000 0000 0000 5010  ..............P.
        0x0030:  0000 0000 0000 7269 6767 6c65 0a         ......riggle.
```