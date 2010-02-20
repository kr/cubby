from unittest import TestCase
from testhelpers import *

class Test(TestCase):
  def setUp(self):
    self.cubby = cubbyd()

  def tearDown(self):
    self.cubby.kill()
    del self.cubby

  def test_putget(self):
    name = '/file/foo'
    data = b'bar'

    self.cubby.http_put(name, data)
    self.assertEqual(self.cubby.http_get(name), data)

  def test_get_missing(self):
    try:
      self.cubby.http_get('/file/not-there')
      self.fail()
    except HTTPError as ex:
      pass

#class TestTwo(TestCase):
#  def setUp(self):
#    self.cubby_a = cubbyd()
#    self.cubby_b = cubbyd(boot=self.cubby_a)
#
#  def test_putget(self):
#    name = '/foo'
#    data = 'bar'
#
#    cubby_a.http_put(name, data)
#    self.assertEqual(cubby_b.http_get(name), data)
