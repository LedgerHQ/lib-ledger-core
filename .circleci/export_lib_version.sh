#!/usr/bin/env bash

function match {
  if [[ "$OSTYPE" == "darwin"* ]]; then
    sed -nE $*
  else
    sed -nr $*
  fi
}

LIB_VERSION_MAJOR=$(cat CMakeLists.txt | match 's/set\(VERSION_MAJOR[^0-9]*([0-9]+).*\)/\1/p')
LIB_VERSION_MINOR=$(cat CMakeLists.txt | match 's/set\(VERSION_MINOR[^0-9]*([0-9]+).*\)/\1/p')
LIB_VERSION_PATCH=$(cat CMakeLists.txt | match 's/set\(VERSION_PATCH[^0-9]*([0-9]+).*\)/\1/p')
LIB_VERSION=$LIB_VERSION_MAJOR.$LIB_VERSION_MINOR.$LIB_VERSION_PATCH

if [[ -z "$CIRCLE_TAG" ]]; then
  if [[ -n "$CIRCLE_SHA1" ]]; then
    COMMIT_HASH=$(echo "$CIRCLE_SHA1" | cut -c 1-6)
  else
    COMMIT_HASH=$(git rev-parse --short HEAD | cut -c 1-6)
  fi
  LIB_VERSION="$LIB_VERSION-rc-$COMMIT_HASH"
fi

echo "export LIB_VERSION=$LIB_VERSION" >> $"BASH_ENV"
echo "$LIB_VERSION"
