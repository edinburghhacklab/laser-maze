#!/usr/bin/env python

import atexit
import serial
import sys
import threading
import time

device = serial.Serial(sys.argv[1], baudrate=57600)

state = 0
changed = False

class SerialReaderThread(threading.Thread):
    def run(self):
        global state
        global changed
        while True:
            line = device.readline()
            if line == '':
                break
            if line.startswith('qreply 10 '):
                state = int(line.strip().split(' ')[2])
                changed = True
            print line,

reader = SerialReaderThread()
reader.daemon = True
reader.start()

device.write(chr(0)+chr(255)+chr(2))
device.flush()

while True:
    if state != 2 and changed == True:
        print "broken"
        time.sleep(1)
        changed = False
        device.write(chr(0)+chr(255)+chr(2))
        device.flush()
        print "reset"
