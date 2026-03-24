#!/usr/bin/env python3

from serial import Serial, SerialException
from sys import stdout, stderr, argv, exit
from time import time
from datetime import datetime

if len(argv) != 3:
    print(f"Usage: {argv[0]} [PORT] [BAUD RATE] > mycapture.btsnoop")
    exit(1)

port = argv[1]
baud = argv[2]

stdout.buffer.write(b"btsnoop\x00")
stdout.buffer.write(0x0001.to_bytes(4, byteorder = 'big'))
stdout.buffer.write((1002).to_bytes(4, byteorder = 'big'))

with Serial(port, baud) as port:
    while True:
        try:
            l = port.readline().decode("ascii").strip()
        except SerialException:
            # Retry once
            l = port.readline().decode("ascii").strip()
        print(l, file=stderr)
        if "RX" not in l and "TX" not in l:
            continue
        parsed = bytes.fromhex(l.split(" ")[-1])
        flags = 0
        if "RX" in l:
            flags = flags | 0x01
        if "ACLDATA" not in l:
            flags = flags | 0x02
        stdout.buffer.write(len(parsed).to_bytes(4, byteorder = 'big'))
        stdout.buffer.write(len(parsed).to_bytes(4, byteorder = 'big'))
        stdout.buffer.write(flags.to_bytes(4, byteorder = 'big'))
        stdout.buffer.write(0x00.to_bytes(4, byteorder = 'big'))
        stdout.buffer.write((0x00E03AB44A676000 + int((datetime.now() - datetime(2000, 1, 1)).total_seconds()*1000000)).to_bytes(8, byteorder = 'big'))
        stdout.buffer.write(parsed)
        stdout.flush()
