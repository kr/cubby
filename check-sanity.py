from unittest import TestCase
from testhelpers import *

class Test(TestCase):
  def test_sanity(self):
    v = cubbyd().http_get_json('/admin/about.json')
    ver = open('version.m4').read()
    self.assertEqual(v['version'], ver)

class TestOne(TestCase):
  def setUp(self):
    self.cubby = cubbyd()

  def tearDown(self):
    self.cubby.kill()
    del self.cubby

  def test_ping(self):
    self.cubby.control_send(Ping,
        memcache_port = 0,
        http_port = 0,
        root_key = b'000000000000',
        chain_len = 1
    )
    response = self.cubby.control_recv()
    self.assertEqual(type(response), Pong)
    self.assertEqual(response.peers[0].addr, self.cubby.tester_control_info[0])
    self.assertEqual(response.peers[0].port, self.cubby.tester_control_info[1])

