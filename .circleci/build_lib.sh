#!/usr/bin/env bash

echo "=====>Create build directory"
pwd
cd .. && mkdir lib-ledger-core-build && cd lib-ledger-core-build

echo "=====>Start build"
pwd
cmake -DCMAKE_INSTALL_PREFIX=/root ../lib-ledger-core && make -j4