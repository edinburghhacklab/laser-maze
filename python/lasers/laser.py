#!/usr/bin/env python
# -*- coding: utf-8 -*-
import enum

import logbook
from tornado import ioloop

from lasers import talker as t

logger = logbook.Logger(__name__)


class NodeState(enum.Enum):
    test = 1
    normal = 2
    broken = 3
    off = 4


def parse_line(line):
    parts = line.strip().split()
    if parts[0] == 'qreply':
        return (int(parts[1]), NodeState(int(parts[2])))
    else:
        return (None, None)


class LaserStateManager(object):
    NODE_RETURN_DELAY = 2

    def __init__(self, auto_return=False):
        self.nodes = {}
        self.laser_broken = lambda i: None  # default to noop, set from outside
        self.n_lasers_broken = lambda: None  # "
        self.n_broken_threshold = 5

        self.auto_return = auto_return

    def game_start(self):
        logger.info('Game Start!')
        self.all_on()
        self.n_broken = 0

    def all_on(self):
        logger.info('Turning all the lasers')
        self.talker.send('\0{}{}'.format(chr(255), chr(2)))
        #for node in self.nodes:
        #    self.talker.send('\0{}{}'.format(chr(node), chr(2)))

    def _laser_on_callback(self, index):
        def laser_on():
            self.talker.send('\0{}{}'.format(chr(index), chr(2)))
        return laser_on

    @property
    def talker(self):
        return self._talker

    @talker.setter
    def talker(self, talker):
        logger.debug('Setting talker')
        self._talker = talker
        talker.data_received = self._receive
        talker.include_self = False

    def _laser_broken(self, index):
        self.n_broken += 1
        self.laser_broken(index)
        if self.n_broken >= self.n_broken_threshold:
            self._n_lasers_broken()

        if self.auto_return:
            ioloop.IOLoop.instance().call_later(self.NODE_RETURN_DELAY,
                                                self._laser_on_callback(index))

    def _n_lasers_broken(self):
        self.n_lasers_broken()

    def _receive(self, line):
        index, state = parse_line(line)
        if index is None:
            logger.debug(line.strip())
        else:
            try:
                old_state = self.nodes[index]
            except KeyError:
                logger.debug('Node {}: {}', index, state)
            else:
                if old_state != state:
                    logger.debug('Node {}: {} -> {}', index, old_state, state)
                if (old_state, state) == (NodeState.normal, NodeState.broken):
                    # laser was just broken
                    logger.info('Laser broken {}', index)
                    self._laser_broken(index)

            self.nodes[index] = state


def run_lasers(serial_device, baudrate, auto_return):
    st = t.SerialTalker(serial_device, baudrate)

    gs = LaserStateManager(auto_return=auto_return)
    gs.talker = st

    gs.laser_broken = lambda i: logger.warn('Laser {} broken!', i)
    gs.n_lasers_broken = lambda: logger.error('Too many lasers broken!!')

    ioloop.IOLoop.instance().call_later(2, gs.game_start)
    ioloop.IOLoop.instance().start()


def main():
    pass

if __name__ == '__main__':
    main()
