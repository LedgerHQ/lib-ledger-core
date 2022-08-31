#!/usr/bin/env bash
set -euo pipefail

LOAD_LIMIT="${BUILD_LOAD_LIMIT:-2.5}"
echo "LOAD_LIMIT==" ${LOAD_LIMIT}

mkdir _build_tests
cd _build_tests
ccache -s
cmake .. -DBUILD_TESTS=ON -DNIX_BUILD=ON -DCCACHE=ON -GNinja
echo "========= Building libcore"
cmake --build . --parallel --target  ledger-core-static
cmake --build . --parallel --target  ledger-core
cmake --build . --parallel
echo "========= Provisioning Database"
initdb -D .tmp/mydb
pg_ctl -D .tmp/mydb -l logfile -o "--unix_socket_directories='$PWD'" start
createdb -p 5432 -h localhost test_db
echo "========= Running tests"
ctest --output-on-failure
ccache -s
