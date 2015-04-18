#!/usr/bin/env python
# -*- coding: utf-8 -*-
import enum

import logbook

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


def GameState(object):
    def __init__(self):
        self.talker = None  # Set externally, API

    @property
    def talker(self):
        return self._talker

    @talker.setter
    def talker(self, talker):
        self._talker = talker
        talker.data_received = self._receive

    def _receive(self, line):
        index, state = parse_line(line)
        if index is not None:
            logger.debug('Node {} in state {}', index, state)


def main():
    pass

if __name__ == '__main__':
    main()
