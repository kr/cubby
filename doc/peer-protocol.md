Here are some algorithms the Cubby nodes (aka peers) use to talk to each
other.

The details aren't fleshed out yet. This is currently just a sketch.

PING:

    reply PING

STORE key len r nc rc dc bc bytes:

    let n = the closest known active node to key

    if n is us:
      if key is already here and complete, reply HAVE key
      allocate len bytes under key in the persistent store
      reply RETRIEVE key 0 0
      set a retrieve timer for the peer
      find appropriate nodes for replication and forward the request

    else:
      let s = the closest N known active nodes to key
      reply SERVERS s

RETRIEVE key offset bitmask:

    if key is here:
      reply DATA
    else:
      let s = the closest N known active nodes to key
      reply SERVERS s

SERVERS server...:

    ...

DATA offset bytes:

    reset the retrieve timer for the peer
    store the block
    forward it to the other nodes
    when we are complete, and bc other nodes are complete, reply HAVE

HAVE key:

    ...

TIMEOUT node-key action blob-key:

    if action is ping:
      ...
    else if action is retrieve:
      let n = node-key
      n <- RETRIEVE blob-key (missing blocks)
