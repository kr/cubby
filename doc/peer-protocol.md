Here are some algorithms the Cubby nodes (aka peers) use to talk to each
other.

The details aren't fleshed out yet. This is currently just a sketch.

PING:

    reply PING

PUT key len w rw bw data:

    if key is already here and complete, reply HAVE key (already)

    allocate len bytes under key in the persistent store
    store the data

    pick bw - 1 servers and forward the request

    find the closest N good nodes to key and send them dirent updates

    reply HAVE key (ok)

    pick w - bw - 1 servers (spanning rw - 1 racks) and forward the request

GET key (maybe with offset and len):

    if key's data is here:
      reply DATA
    else if we have the dirent D:
      let S = the least-loaded server in D
    else if we don't know anything:
      let S = the closest N known active nodes to key

    if the client supports redirects:
      reply SERVERS S
    else:
      proxy the request to one of S

B <- LINK(key=K, targets=T, rank=R) <- A

    if R < NUM_LINKS:
      R' = our implicit rank
      if (R' == R):
        store entry for T under key K at rank R
        C = next closest node
        LINK(K, T, R + 1) -> C
        when LINKED(K) <- C
          LINKED(K) -> A
      else if R' > R: xxx
      else if R' < R: yyy
    else if we have an entry for K:
      delete entry under key K
      C = next closest node
      LINK(K, T, R + 1) -> C
      when LINKED(K) <- C
        LINKED(K) -> A
    else:
      LINKED(K) -> A

B <- LINKED(key=K) <- A

    run callbacks
