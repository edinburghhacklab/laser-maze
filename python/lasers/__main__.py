#!/usr/bin/env python
# -*- coding: utf-8 -*-
import click
import logbook
import logbook.more

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
@click.pass_context
def talk(ctx):
    talker.talk(ctx.obj['serial_device'], ctx.obj['baudrate'])


def main():
    root(obj={})

if __name__ == '__main__':
    main()
