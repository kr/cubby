import json
import time
import tempfile
import subprocess

class HTTPError(Exception):
  def __init__(self, response, path):
    self.args = (response.status, response.reason, path)
    self.response = response

class cubbyd_runner:
  def __init__(self,
      addr='127.0.0.1',
      http_port='auto',
      bundles='auto',
      init=True):
    self.args = ('./cubbyd',)

    if addr:
      self.addr = addr
      self.args += ('-l', addr)

    if http_port:
      if http_port is 'auto': http_port = get_free_tcp_port()
      self.http_port = http_port
      self.args += ('-p', str(http_port))

    if bundles:
      if bundles is 'auto': bundles = (make_bundle(),)
      self.bundles = bundles
      for b in bundles:
        if type(b) == type(''):
          name = b
        else:
          name = b.name
        self.args += ('-f', name)

    self.init = not not init
    if init:
      self.args += ('-i',)

  def __del__(self):
    if hasattr(self, 'process'):
      self.process.kill()

  def run(self, more_args=()):
    self.stdin = tempfile.TemporaryFile()
    self.stdout = tempfile.TemporaryFile()
    self.stderr = tempfile.TemporaryFile()
    self.process = subprocess.Popen(self.args + more_args,
        stdin=self.stdin,
        stdout=self.stdout,
        stderr=self.stderr)

    # This is to prevent requests to the new daemon from being issued before it
    # has opened the network ports. TODO make this more robust.
    time.sleep(0.1)

    return self

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
  return 12345

def make_bundle(size=10485760, init=b''):
  f = tempfile.NamedTemporaryFile()
  f.write(init)
  f.seek(size - 1)
  f.write(b'\0')
  f.seek(0)
  return f
