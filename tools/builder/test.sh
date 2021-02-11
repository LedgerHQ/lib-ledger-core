#!/bin/sh
TARGET=$1
shift

make $TARGET && ctest "$@"
