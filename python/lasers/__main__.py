#!/usr/bin/env python
# -*- coding: utf-8 -*-
import click
import logbook
import logbook.more

from lasers import game
from lasers import talker


@click.group()
@click.argument('serial_device')
@click.option('--baudrate', '-b', type=int, default=57600)
@click.option('--debug/--no-debug', '-d/-D', default=False)
@click.pass_context
def root(ctx, serial_device, baudrate, debug):
    ctx.obj['serial_device'] = serial_device
    ctx.obj['baudrate'] = baudrate

    log_handler = logbook.more.ColorizedStderrHandler(
        format_string='{record.time:%Y-%m-%d %H:%M:%S.%f} '
        '{record.level_name:.1} {record.channel}:{record.lineno}: '
        '{record.message}', bubble=False)
    if debug:
        log_handler.level = logbook.DEBUG
    else:
        log_handler.level = logbook.WARNING
    logbook.NullHandler().push_application()
    log_handler.push_application()


@root.command()
@click.pass_obj
def talk(obj):
    talker.talk(obj['serial_device'], obj['baudrate'])


@root.command()
@click.option('--auto-return/--no-auto-return', default=True, help='Turn the lasers back on after 2s')
@click.pass_obj
def run_game(obj, auto_return):
    game.run_game(obj['serial_device'], obj['baudrate'], auto_return)


def main():
    root(obj={})

if __name__ == '__main__':
    main()
