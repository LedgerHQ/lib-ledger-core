#!/usr/bin/env bash
set -euo pipefail

LOAD_LIMIT="${BUILD_LOAD_LIMIT:-2.5}"
echo "LOAD_LIMIT==" ${LOAD_LIMIT}

apt-get update && apt-get install ninja-build  # TODO: move it to build env

mkdir _build_tests
cd _build_tests
cmake .. -DBUILD_TESTS=ON -DNIX_BUILD=OFF -DCCACHE=ON -DSYS_OPENSSL=ON -DOPENSSL_USE_STATIC_LIBS=TRUE -DPOSTGRES_HOST=$POSTGRES_HOST -DPOSTGRES_PORT=$POSTGRES_PORT -GNinja
echo "========= Building libcore"
cmake --build . --parallel --target  ledger-core-static
cmake --build . --parallel --target  ledger-core
cmake --build . --parallel
echo "========= Running tests"
ctest --output-on-failure
