#!/usr/bin/env python
# -*- coding: utf-8 -*-
from setuptools import setup

setup(name='hacklab-lasers',
      author='KitB',
      author_email='kit@ninjalith.com',
      packages=['lasers'],
      zip_safe=False,
      install_requires=[
          'Logbook',
          'click',
          'enum34',
          'pyserial',
          'tornado',
      ],
      scripts=['bin/lasers'],
      )
