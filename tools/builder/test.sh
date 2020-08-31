#!/bin/sh
TARGET=$1

make $TARGET && ctest
