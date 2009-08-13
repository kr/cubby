from unittest import TestCase
from testhelpers import *

class Test(TestCase):
  def setUp(self):
    self.c = cubbyd()

  def test_putget(self):
    name = '/foo'
    data = b'bar'

    self.c.http_put(name, data)
    self.assertEqual(self.c.http_get(name), data)

#class TestTwo(TestCase):
#  def setUp(self):
#    self.a = cubbyd()
#    self.b = cubbyd(boot=self.a)
#
#  def test_putget(self):
#    name = '/foo'
#    data = 'bar'
#
#    a.http_put(name, data)
#    self.assertEqual(b.http_get(name), data)
