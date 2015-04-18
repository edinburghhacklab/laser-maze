#!/usr/bin/env python

import atexit
import serial
import sys
import threading
import time

device = serial.Serial(sys.argv[1], baudrate=57600)
max_device = 20
seen = {}

class SerialReaderThread(threading.Thread):
    def run(self):
        global seen
        while True:
            line = device.readline()
            if line == '':
                break
            if line.startswith('qreply '):
                txt, node, state = line.strip().split(' ')
                seen[int(node)] = time.time()

reader = SerialReaderThread()
reader.daemon = True
reader.start()

while True:
    sys.stdout.write('\r')
    for node in range(1, max_device+1):
        if seen.has_key(node) and time.time()-seen[node] < 0.5:
            sys.stdout.write("%02d " % (node))
        else:
            sys.stdout.write("-- ")
    sys.stdout.flush()
    time.sleep(0.1)
