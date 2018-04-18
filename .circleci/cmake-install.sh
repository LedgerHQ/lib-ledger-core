#!/usr/bin/env bash
echo "========> Install cmake"
cd /root
wget --quiet https://cmake.org/files/v3.10/cmake-3.10.3.tar.gz && tar -xvf cmake-3.10.3.tar.gz
#mv cmake-* cmake
pwd
ls -la
cd cmake-3.10.3
ls -la
./bootstrap
make
make install