#!/bin/bash
# 
# compile and run server
# send 3 test commands
#   ADD 1, 3
#   COUNT 1,3
#   EXIT
# 
# pipe output to hexdump

PORT="7890"

make 2&>/dev/null

echo "=== Starting Server on Port $PORT ==="
./bin/server $PORT &
SERVER_PID=$!

sleep 1

echo "=== Running Test Operations ==="

    perl -e 'print "\x08\x00\x02\x00\x00\x00\x01\x00\x00\x00\x03";
        print "\x09\x01\x02\x00\x00\x00\x01\x00\x00\x00\x03";
        print "\x0A\x02"' \
        | nc localhost $PORT | hexdump -C

sleep 2
