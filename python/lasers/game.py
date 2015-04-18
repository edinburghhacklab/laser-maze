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


class Node(object):
    def __init__(self, index):
        self.index = index


class GameState(object):
    def __init__(self):
        self.nodes = {}

    @property
    def talker(self):
        return self._talker

    @talker.setter
    def talker(self, talker):
        logger.debug('Setting talker')
        self._talker = talker
        talker.data_received = self._receive
        talker.include_self = False

    def _receive(self, line):
        index, state = parse_line(line)
        if index is not None:
            logger.debug('Node {} in state {}', index, state)
            self.nodes[index] = state


def run_game(serial_device, baudrate):
    st = t.SerialTalker(serial_device, baudrate)

    gs = GameState()
    gs.talker = st

    ioloop.IOLoop.instance().start()


def main():
    pass

if __name__ == '__main__':
    main()
