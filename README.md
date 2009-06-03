# Cubby Readme

Store billions of photos easily and efficiently.

Cubby is a distributed key-value store for static blobs. It is simple, fast,
scalable, and tuned for the datacenter.

It is ideal for storing thumbnails, photos, videos, and other medium to large
files that never change.

# Easy to Use

Cubby automatically manages new nodes, failures, and routing.

To add more storage, you just turn on a new box and fire up cubbyd. No
configuration is necessary. If a node fails, Cubby will re-replicate files to
maintain your desired level of reduncancy.

# Scalable

Cubby is designed to scale to hundreds of nodes and petabytes of storage.

In the future, if there is demand, we may extend Cubby to use a [Distributed
Hash Table][DHT] for routing requests. This would allow the system to scale to
millions of nodes and exabytes of storage.

# Fast

Cubby stores blobs in an efficient, packed format. It causes at most one disk
seek for each file read or written.

This is similar to the technique used in [Varnish][] and [Haystack][].

# Tuned for the Datacenter

We assume the local network is fast, but (of course) we do not assume that the
network or hardware is reliable.

We assume that Cubby nodes are responsible citizens; and that clients and peers
are never malicious.

We also assume you have plenty of read caching in front of your Cubby cluster.
Thus our performance priorities are, roughly in order: minimizing write
latency, minimizing jitter in write latency, and maximising throughput. Read
performance usually takes a back seat.

These assumptions enable design decisions that provide better performance.

# Get Involved

We're just getting started, so there's no mailing list yet. Please send any
questions or comments to Keith Rarick <kr@xph.us>.

[DHT]: http://en.wikipedia.org/wiki/Distributed_hash_table
[Varnish]: http://varnish.projects.linpro.no/
[Haystack]: http://www.facebook.com/note.php?note_id=76191543919
