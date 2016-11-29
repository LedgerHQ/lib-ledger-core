#!/usr/bin/env bash

mkdir tools/mingw64_tmp/ && cp tools/mingw64/* tools/mingw64_tmp/
cd tools/mingw64_tmp
for file in *.xz; do
    tar xf ${file}
    sudo rsync -lrK usr /
done
