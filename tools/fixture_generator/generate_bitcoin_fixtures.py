#!/usr/bin/python

import sys
import httplib

if len(sys.argv) != 3:
    print "Should have exactly two params <name> <xpub>"
    sys.exit(-1)


name = sys.argv[1]
xpub = sys.argv[2]

conn = httplib.HTTPSConnection("https://blockchain.info/")
conn.request("GET", "/fr/multiaddr?active=")
r1 = conn.getresponse().read()
print r1.read()

print sys.argv[1]

