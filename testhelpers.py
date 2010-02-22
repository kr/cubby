import json
import time
import tempfile
import subprocess
import socket
import random
from struct import Struct
from collections import namedtuple

DEV_NULL = open('/dev/null', 'w')

class HTTPError(Exception):
  def __init__(self, response, path):
    self.args = (response.status, response.reason, path)
    self.response = response

class cubbyd_runner:
  _tester_control_port = None
  _control_socket = None

  def __init__(self,
      addr='127.0.0.1',
      http_port='auto',
      memcache_port='auto',
      control_port='auto',
      bundles='auto',
      boot=None,
      init=True):
    self.program = ('./cubbyd',)
    self.opts = ()
    self.files = ()

    self.addr = addr

    if http_port is 'auto':
      http_port = get_free_tcp_port()
    self.http_port = http_port

    if memcache_port is 'auto':
      memcache_port = get_free_tcp_port()
    self.memcache_port = memcache_port

    if control_port is 'auto':
      control_port = get_free_udp_port()
    self.control_port = control_port

    self.opts += ('-l', addr)
    self.opts += ('-p', str(http_port))
    self.opts += ('-m', str(memcache_port))
    self.opts += ('-c', str(control_port))

    if bundles:
      if bundles is 'auto': bundles = (make_bundle(),)
      self.bundles = bundles
      for b in bundles:
        if type(b) == type(''):
          name = b
        else:
          name = b.name
        self.files += (name,)

    self.init = not not init
    if init:
      self.opts += ('-i',)

    if boot:
      self.boot_info = boot.addr, boot.control_port
      self.opts += ('-b', '%s:%d' % self.boot_info)

  def __del__(self):
    self.kill()
    self.unlink_bundles()

  def unlink_bundles(self):
    for b in self.bundles:
      b.close()

  def kill(self):
    if hasattr(self, 'process'):
      self.process.kill()
      del self.process

  def run(self, more_opts=(), more_files=()):
    self.stdin = tempfile.TemporaryFile()
    self.stdout = tempfile.TemporaryFile()
    self.stderr = tempfile.TemporaryFile()

    self.process = subprocess.Popen(
        self.program + self.opts + more_opts + self.files + more_files,
        stdin=self.stdin,
        stdout=self.stdout,
        stderr=self.stderr)

    # Wait until it gets fully started up.
    while not self.is_running:
      time.sleep(0.05)

    return self

  # Check if the process has opened its network ports. If so, we declare it to
  # be running.
  @property
  def is_running(self):
    #lsof_addr = 'TCP@%s:%d' % (self.addr, self.http_port)
    lsof_addr = 'TCP:%d' % (self.http_port,)
    retcode = subprocess.call(['lsof', '-i', lsof_addr], stdout=DEV_NULL)
    return retcode == 0

  def http_get(self, path):
    import http.client
    conn = http.client.HTTPConnection(self.addr, self.http_port)
    try:
      conn.request("GET", path)
      response = conn.getresponse()
      if (response.status != 200):
        raise HTTPError(response, path)
      return response.read()
    finally:
      conn.close()

  def http_put(self, path, data):
    import http.client
    conn = http.client.HTTPConnection(self.addr, self.http_port)
    try:
      conn.request("PUT", path, body=data)
      response = conn.getresponse()
      if (response.status != 200):
        raise HTTPError(response, path)
      return response.read()
    finally:
      conn.close()

  def http_get_json(self, *args, **kwargs):
    return json.loads(self.http_get(*args, **kwargs).decode())

  @property
  def control_info(self):
    return self.addr, self.control_port

  @property
  def tester_control_info(self):
    if not self._tester_control_port:
      self._tester_control_port = get_free_udp_port()
    return '127.0.0.1', self._tester_control_port

  @property
  def control_socket(self):
    if not self._control_socket:
      self._control_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
      self._control_socket.bind(self.tester_control_info)
    return self._control_socket

  def udp_send(self, payload, timeout=5):
    self.control_socket.settimeout(timeout)
    self.control_socket.sendto(payload, (self.addr, self.control_port))

  def udp_recv(self, timeout=5):
    self.control_socket.settimeout(timeout)
    payload, addr = self.control_socket.recvfrom(2**16)
    assert(addr == (self.addr, self.control_port))
    return payload

  def control_send(self, type, **kw):
    packet = type(type=type.type, **kw)
    self.udp_send(packet.bytes)

  def control_recv(self):
    return unpack_control_packet(self.udp_recv())

def cubbyd(*args, **kwargs):
  'Start cubbyd with the given options'

  return cubbyd_runner(*args, **kwargs).run()

def get_free_tcp_port():
  return random.randrange(10000, 50000)

def get_free_udp_port():
  return random.randrange(10000, 50000)

def make_bundle(size=10485760, init=b''):
  f = tempfile.NamedTemporaryFile()
  f.write(init)
  f.seek(size - 1)
  f.write(b'\0')
  f.seek(0)
  return f

def group_bytes(b, n):
  groups = []
  for i, x in enumerate(b):
    if i % n == 0:
      groups.append(b'')
    groups[-1] += bytes([x])
  return groups

def formatter(name, fmt, fields):
  class Formatter(namedtuple(name, fields)):
    __slots__ = ()
    format = Struct(fmt)
    @property
    def bytes(self):
      return self.format.pack(*self)
    @classmethod
    def unpack(clas, bytes):
      main, rest = bytes[:clas.format.size], bytes[clas.format.size:]
      return clas.make(clas.format.unpack(main)).unpack_extra(rest)
    def unpack_extra(self, bytes):
      assert len(bytes) == 0
      return self
    @classmethod
    def make(clas, *args, **kw):
      return clas._make(*args, **kw)

  Formatter.__name__ = name
  return Formatter

class Peer(formatter('Peer', '>4sHxx', 'n_addr port')):
  __slots__ = ()

  @classmethod
  def __new__(clas, *args, **kw):
    if 'addr' in kw:
      addr = kw['addr']
      del kw['addr']
      kw['n_addr'] = socket.inet_aton(addr)
      print(args, kw)
    return super(clas).__new__(*args, **kw)

  @property
  def addr(self):
    return socket.inet_ntoa(self.n_addr)

  def __repr__(self):
    return 'Peer(addr=%r, port=%r)>' % (self.addr, self.port)

Ping = formatter('Ping', '>B7xHH12sH6x', 'type memcache_port http_port root_key chain_len')

class Pong(formatter('Pong', '>B7x12sH2x', 'type root_key chain_len')):
  def unpack_extra(self, bytes):
    assert len(bytes) % Peer.format.size == 0
    self.peers = list(map(Peer.unpack, group_bytes(bytes, Peer.format.size)))
    return self

class Link(formatter('Link', '>B7x12s3xB', 'type key rank')):
  def unpack_extra(self, bytes):
    assert len(bytes) % Peer.format.size == 0
    self.targets = list(map(Peer.unpack, group_bytes(bytes, Peer.format.size)))
    return self

Linked = formatter('Linked', '>B7x12s4x', 'type key')

packet_types = [
  Ping,
  Pong,
  Link,
  Linked,
]

for i, ptype in enumerate(packet_types):
  ptype.type = i

def unpack_control_packet(bytes):
  type = bytes[0]
  return packet_types[type].unpack(bytes)

