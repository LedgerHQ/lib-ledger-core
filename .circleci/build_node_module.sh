#!/usr/bin/env bash

echo "=====>Build node module"

cd ledger-core-samples/nodejs
yarn
mkdir tmp
node tests/basic-test.js
