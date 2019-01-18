#!/usr/bin/env bash
echo "========> Install CMake"
cd $HOME
wget --quiet https://cmake.org/files/v3.12/cmake-3.12.3.tar.gz && tar -xvf cmake-3.12.3.tar.gz
cd cmake-3.12.3