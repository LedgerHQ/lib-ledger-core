#!/usr/bin/env bash
set -euo pipefail

LOAD_LIMIT="${BUILD_LOAD_LIMIT:-2.5}"

mkdir _build_tests
cd _build_tests
cmake .. -DSYS_OPENSSL=ON -DSYS_SECP256K1=ON -DPG_SUPPORT=ON -DBUILD_TESTS=ON
echo "========= Building libcore"
make -j -l"${LOAD_LIMIT}" ledger-core-static
make -j -l"${LOAD_LIMIT}" ledger-core
make -j -l"${LOAD_LIMIT}"
echo "========= Provisioning Database"
initdb -D .tmp/mydb
pg_ctl -D .tmp/mydb -l logfile -o "--unix_socket_directories='$PWD'" start
createdb -p 5432 -h localhost test_db
echo "========= Running tests"
ctest --output-on-failure
