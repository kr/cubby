#!/usr/bin/env bash

set -e
set -x

cleanup() {
    kill $pid1 || true
    kill $pid2 || true
}

trap cleanup EXIT

if test "$1" = clean
then
    rm -f /tmp/x.bun /tmp/y.bun
    dd if=/dev/zero of=/tmp/x.bun bs=1k count=128k
    dd if=/dev/zero of=/tmp/y.bun bs=1k count=128k
fi

./cubbyd -p 8000 -m 11211 -c 20000 -f /tmp/x.bun -i &
pid1=$!

sleep 0.2

./cubbyd -p 8001 -m 11212 -c 20001 -f /tmp/y.bun -i -b 127.0.0.1:20000 &
pid2=$!

sleep 0.2

wait
wait
