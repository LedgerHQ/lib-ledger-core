#!/usr/bin/env bash

# Build for the current OS
mkdir build && cd build && cmake .. && make && make test && cd ..

if [ "$TRAVIS_OS_NAME" == "linux" ]; then
    # Build for Windows x86
    mkdir build_win && cd build_win && cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchains/Toolchain-mingw64.cmake && make && cd ..;

    # Build for Windows x64

    # Build for Linux x86

    # Build for Linux x64

elif [ "$TRAVIS_OS_NAME" == "osx" ]; then
    # Build for Android

    # Build for iOS

    # Build OSX
fi
