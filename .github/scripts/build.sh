#!/usr/bin/env bash
set -euo pipefail

mkdir _build_tests
cd _build_tests
cmake .. -DTARGET_JNI=${BUILD_JNI-OFF} -DBUILD_TESTS=ON -DNIX_BUILD=OFF -DCCACHE=ON -DSYS_OPENSSL=ON -DOPENSSL_USE_STATIC_LIBS=TRUE ${ARGS}
echo "========= Building libcore"
cmake --build . --target  ledger-core-static -- -j2
cmake --build . --target  ledger-core -- -j2
cmake --build . -- -j2