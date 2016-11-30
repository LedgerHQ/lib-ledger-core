#!/usr/bin/env bash

mkdir tools/mingw64_tmp/ && cp tools/mingw64/* tools/mingw64_tmp/
cd tools/mingw64_tmp
wget https://api.ledgerwallet.com/assets/sources/mingw-w64-gcc-6.2.1-1-x86_64.pkg.tar.xz
for file in *.xz; do
    tar xf ${file}
done

sudo rsync -lrK usr /git