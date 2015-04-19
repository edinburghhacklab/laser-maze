#!/usr/bin/env python
# -*- coding: utf-8 -*-
import logbook

from tornado import ioloop

from lasers import talker
from lasers import tbrb
from lasers import laser

logger = logbook.Logger(__name__)


class Game(object):
    def __init__(self, laser_state_manager, button_talker):
        self.laser_state_manager = laser_state_manager
        self.laser_state_manager.laser_broken = self.laser_broken
        self.laser_state_manager.n_lasers_broken = self.n_lasers_broken

        self.button_talker = button_talker
        self.button_talker.on_press = self.button_pressed

        self.start_time = None

    def new_game(self):
        """ Reinitialise game state. """

    def start_game(self):
        """ Turn the lasers on and start the timer, probably with a countdown """
        self.laser_state_manager.game_start()

    def laser_broken(self, index):
        logger.warn('Done broke a laser!')

    def n_lasers_broken(self):
        logger.error('Broke so many lasers! Failfailfail')

    def button_pressed(self):
        logger.notice('Success!')


def run_game(serial_device, baudrate, auto_return):
    st = talker.SerialTalker(serial_device, baudrate)

    lsm = laser.LaserStateManager(auto_return=auto_return)
    lsm.talker = st

    the_button = tbrb.get_tbrb()
    bm = tbrb.ButtonTalker(the_button)
    bm.start()

    game = Game(lsm, bm)
    game.new_game()

    starter = ioloop.PeriodicCallback(lsm.all_on, 3000)
    starter.start()

    ioloop.IOLoop.instance().call_later(2, game.start_game)
    ioloop.IOLoop.instance().start()


def main():
    pass

if __name__ == '__main__':
    main()
