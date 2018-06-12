#!/usr/bin/env bash

cd ledger-core-samples/nodejs
yarn
mkdir tmp
node tests/wallet-pool-test.js
