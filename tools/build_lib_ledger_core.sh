cd .. && mkdir lib-ledger-core-build && cd lib-ledger-core-build

cmake -DCMAKE_INSTALL_PREFIX=/usr/local/Cellar/qt/5.10.0_1 ../lib-ledger-core && make
