# -*- coding: utf-8 -*-

# Author: David Manouchehri <manouchehri@protonmail.com>
# This script will always echo back data on the UDP port of your choice.
# Useful if you want nmap to report a UDP port as "open" instead of "open|filtered" on a standard scan.
# Works with both Python 2 & 3.

import socket
import sys

from datetime import datetime

if len(sys.argv) < 2:
        sys.stderr.write('Usage: uping <port>\n')
        sys.exit(1)
        
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

server_address = '0.0.0.0'
server_port = int(sys.argv[1])

server = (server_address, server_port)
sock.bind(server)
print("Listening on " + server_address + ":" + str(server_port))

TIMEFORMAT='%H:%M:%S'

while True:
        payload, client_address = sock.recvfrom(1024)
        now = datetime.now()
        timestampstr = now.strftime(TIMEFORMAT)
        seqno = payload[0] + payload[1]*256
        reply = payload[:15]
        
        print(f"{[timestampstr]} {seqno}: {len(reply)} bytes " + str(client_address))
        
        sent = sock.sendto(reply, client_address)
