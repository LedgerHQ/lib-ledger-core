#!/usr/bin/env bash
echo "========> Install CMake"
cd $HOME
wget --quiet https://cmake.org/files/v3.10/cmake-3.10.3.tar.gz && tar -xvf cmake-3.10.3.tar.gz
cd cmake-3.10.3
./bootstrap
make
make install