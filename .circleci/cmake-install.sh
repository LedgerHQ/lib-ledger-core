#!/usr/bin/env bash



echo "========> Install CMake"
cd $HOME
wget --quiet https://github.com/Kitware/CMake/releases/download/v3.16.5/cmake-3.16.5-Linux-x86_64.tar.gz && tar -xf cmake-3.16.5-Linux-x86_64.tar.gz
ln -s cmake-3.16.5-Linux-x86_64 cmake_folder