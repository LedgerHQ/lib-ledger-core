#!/usr/bin/env bash

echo "Building external dependencies"

cd externals
rm -rf build
mkdir build

echo "Building secp256pk1"

cd secp256k1
./autogen.sh && ./configure && make && cp .libs/libsecp256k1.a ../build/

