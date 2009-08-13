#!/usr/bin/env python3

import unittest
from glob import glob

def load(x):
  return unittest.defaultTestLoader.loadTestsFromTestCase(__import__(x).Test)

s = unittest.TestSuite([ load(name[:-3]) for name in glob('check-*.py') ])
unittest.TextTestRunner().run(s)
