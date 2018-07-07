#!/usr/bin/env bash

echo "=====>Build node module"

cd ledger-core-samples/nodejs
yarn
mkdir tmp
node tests/basic-test.js

LIB_VERSION=$(node tests/lib-version.js)
echo "export LIB_VERSION=$LIB_VERSION" >> $BASH_ENV

echo "=====> Libcore version"
echo $LIB_VERSION