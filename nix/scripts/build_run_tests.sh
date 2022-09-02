#!/usr/bin/env bash
set -euo pipefail

mkdir _build_tests
cd _build_tests
cmake .. -DBUILD_TESTS=ON -DNIX_BUILD=OFF -DCCACHE=ON -DSYS_OPENSSL=${SYS_OPENSSL-ON} -DOPENSSL_USE_STATIC_LIBS=${OPENSSL_USE_STATIC_LIBS-TRUE} -DPOSTGRES_HOST=$POSTGRES_HOST -DPOSTGRES_PORT=$POSTGRES_PORT -GNinja
echo "========= Building libcore"
cmake --build . --parallel --target  ledger-core-static
cmake --build . --parallel --target  ledger-core
cmake --build . --parallel
echo "========= Running tests"
ctest --output-on-failure
