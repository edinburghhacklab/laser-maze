#!/usr/bin/env python
# -*- coding: utf-8 -*-
""" The Big Red Button """
import evdev
import logbook
from tornado import ioloop

logger = logbook.Logger(__name__)


def get_tbrb():
    devices = [evdev.InputDevice(fd) for fd in evdev.list_devices()]
    for d in devices:
        if d.phys == 'usb-0000:00:14.0-2/input2':
            return d


class ButtonTalker(object):
    def __init__(self, input_device):
        self.input_device = input_device
        self.pressed = False

        self.on_press = lambda: None

        loop_inst = ioloop.IOLoop.instance()
        loop_inst.add_handler(self.input_device.fd, self._event, ioloop.IOLoop.READ)

    def start(self):
        self.input_device.grab()

    def _event(self, *args):
        list(self.input_device.read())  # consume the events so they're gone
        if not self.pressed:  # bouncy bounce
            self.pressed = True
            ioloop.IOLoop.instance().call_later(1, self._unpress)
            self.on_press()

    def _unpress(self):
        self.pressed = False


def main():
    tbrb = get_tbrb()
    bt = ButtonTalker(tbrb)
    bt.on_press = lambda: logger.debug('Done pressed!')
    bt.start()

    ioloop.IOLoop.instance().start()

if __name__ == '__main__':
    main()
