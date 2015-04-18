#!/usr/bin/env python

import atexit
import serial
import sys
import threading
import time

device = serial.Serial(sys.argv[1], baudrate=57600)

class SerialReaderThread(threading.Thread):
    def run(self):
        while True:
            line = device.readline()
            if line == '':
                break
            print line,

def exit_handler():
    device.flush()
    device.write(chr(0)+chr(255)+chr(3))
    device.flush()
    device.write(chr(0)+chr(255)+chr(3))
    device.flush()

reader = SerialReaderThread()
reader.daemon = True
reader.start()

atexit.register(exit_handler)

while True:
    device.write(chr(0)+chr(255)+chr(4))
    device.flush()
