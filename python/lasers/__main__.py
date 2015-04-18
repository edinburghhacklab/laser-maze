#!/usr/bin/env python
# -*- coding: utf-8 -*-
import click

from lasers import talker


@click.group()
@click.argument('serial_device')
@click.option('--baudrate', '-b', type=int, default=57600)
@click.pass_context
def root(ctx, serial_device, baudrate):
    ctx.obj['serial_device'] = serial_device
    ctx.obj['baudrate'] = baudrate


@root.command()
@click.pass_context
def talk(ctx):
    talker.talk(ctx.obj['serial_device'], ctx.obj['baudrate'])


def main():
    root(obj={})

if __name__ == '__main__':
    main()
