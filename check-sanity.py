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
    got_peers = sorted([(x.addr, x.port) for x in response.peers])
    exp_peers = sorted((self.cubby.tester_control_info, self.cubby.control_info))
    self.assertEqual(got_peers, exp_peers)

class TestMember(TestCase):
  def setUp(self):
    self.root_key = b'kkkkkkkkkkkk'
    self.cubby = cubbyd()
    self.cubby.control_send(Ping,
        memcache_port = 0,
        http_port = 0,
        root_key = self.root_key,
        chain_len = 1
    )
    self.pong = self.cubby.control_recv()

  def tearDown(self):
    self.cubby.kill()
    del self.cubby


  def test_link_secondary_last(self):
    key = bytearray(self.root_key)
    key[-1] = (key[-1] + 1) % 256
    self.cubby.control_send(Link,
        key = key,
        rank = 1,
    )
    response = self.cubby.control_recv()
    self.assertEqual(type(response), Linked)

  def test_link_primary(self):
    key = bytearray(self.pong.root_key)
    key[-1] = (key[-1] + 1) % 256
    self.cubby.control_send(Link, key = key, rank = 0)
    response = self.cubby.control_recv()
    self.assertEqual(type(response), Link)
    self.assertEqual(response.key, key)
    self.assertEqual(response.rank, 1)

    self.cubby.control_send(Linked, key = key)
    response = self.cubby.control_recv()
    self.assertEqual(type(response), Linked)
    self.assertEqual(response.key, key)
