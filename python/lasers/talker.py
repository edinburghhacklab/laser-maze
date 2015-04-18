#!/usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import print_function

import logbook
import serial
from tornado import ioloop

logger = logbook.Logger(__name__)


class SerialTalker(object):
    def __init__(self, serial_port, baud_rate, include_self=False):
        self.serial_file = serial.Serial(serial_port, baud_rate, timeout=0.1)
        self.serial_file.flushInput()
        self.serial_file.flushOutput()

        self.include_self = include_self
        self.data_received = None  # Set externally, that's the API

        loop_inst = ioloop.IOLoop.instance()
        loop_inst.add_handler(self.serial_file, self._data_received, ioloop.IOLoop.READ)

    def _data_received(self, *args):
        line = self.serial_file.readline()
        if self.include_self:
            self.data_received(self, line)
        else:
            self.data_received(line)

    def send(self, message):
        """ Send over serial. """
        logger.debug("Serial Sending {!r}".format(message))
        self.serial_file.write(message)


def talk(serial_device, baudrate):
    st = SerialTalker(serial_device, baudrate)

    def output(d):
        print(d)

    st.data_received = output

    ioloop.IOLoop.instance().start()


def main():
    pass

if __name__ == '__main__':
    main()
