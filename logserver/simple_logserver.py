#!/usr/bin/env python

import socket
import sys

log_ip   = '0.0.0.0'
log_port = 7890

def main():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.bind((log_ip, log_port))

    while True:
        data, addr = s.recvfrom(1024)
        sys.stdout.write(data)

if __name__ == '__main__':
    if len(sys.argv) > 1:
        log_port = int(sys.argv[1])
    main()
