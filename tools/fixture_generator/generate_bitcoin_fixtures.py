#!/usr/bin/python

import sys
import httplib
import json
from pprint import pprint

def writeFixturesHeaderFile():
    print "youhou"

def writeFixturesBodyFile():
    print "YOHO"

if len(sys.argv) != 3:
    print "Should have exactly two params <name> <xpub>"
    sys.exit(-1)


name = sys.argv[1]
xpub = sys.argv[2]

conn = httplib.HTTPSConnection("blockchain.info")
conn.request("GET", "/fr/multiaddr?active=" + xpub)
r1 = conn.getresponse()
data = json.load(r1)

pprint(data)

print sys.argv[1]

