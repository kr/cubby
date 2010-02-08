from unittest import TestCase
from testhelpers import *

class Test(TestCase):
  def test_sanity(self):
    v = cubbyd().http_get_json('/$/debug.json')
    ver = open('version.m4').read()
    self.assertEqual(v['version'], ver)
