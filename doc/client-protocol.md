# Cubby Protocol

We distinguish between "peers" and "clients". Peers (sometimes referred to as
"nodes") are instances of cubbyd; they provide storage and communicate with
one another and the outside world. Clients talk to peers and make use of the
Cubby cluster.

This document does not describe the protocol used by Cubby nodes in
communicating with each other; Cubby peers have more complicated requirements.
See doc/peer-protocol.md for that documentation. Clients only need to follow
the description given here.

## Overview

Cubby has two main parts: physical storage of blobs in arbitrary locations and
a distributed directory mapping names to storage locations.

The details of Cubby’s on-the-wire format are not finalized yet. We’re
currently thinking of using the memcache protocol and/or HTTP. Regardless,
Cubby will provide to clients two basic operations:

    * put name blob → okay, error
    * get name → redirect, blob, error

A client wishing to store a blob in Cubby simply picks a server and sends it a
put command.

A client wishing to retrieve a blob b identified by key k uses consistent
hashing to select the appropriate node q, then issues a get command. In the
unlikely event that q contains b, it will return b directly. (For N nodes, this
should happen for 3/N2 of requests.) Otherwise, q is expected to have the
directory entry for k; then q will return a redirect indicating the storage
location of b.

Clients are expected to maintain a full list of storage nodes in memory. This
list can be retrieved and updated from any node, either by a special command or
by fetching a special name. This list will include metadata about the nodes,
including load and free space. When storing a blob, the client should try to
pick a server with sufficient capacity (both storage and load) to handle the
request. We can play with various heuristics for load-leveling and

Keys are 128 bits long. The key for a name is calculated as the first 128
bits of the SHA-256 hash of the name.

## Format

[stub]
