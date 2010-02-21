import json
import time
import tempfile
import subprocess
import random

DEV_NULL = open('/dev/null', 'w')

class HTTPError(Exception):
  def __init__(self, response, path):
    self.args = (response.status, response.reason, path)
    self.response = response

class cubbyd_runner:
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
