#!/usr/bin/env python3

import unittest
from glob import glob

def load(name):
  return unittest.defaultTestLoader.loadTestsFromModule(__import__(name))

s = unittest.TestSuite([ load(name[:-3]) for name in glob('check-*.py') ])
unittest.TextTestRunner().run(s)
