#!/usr/bin/env bash
set -euo pipefail

mkdir _build_tests
cd _build_tests
cmake .. -DCMAKE_BUILD_TYPE=${BUILD_TYPE-Release} -DTARGET_JNI=${BUILD_JNI-OFF} -DBUILD_TESTS=ON -DNIX_BUILD=OFF -DCCACHE=ON -DSYS_OPENSSL=ON -DOPENSSL_USE_STATIC_LIBS=TRUE -GNinja
echo "========= Building libcore"
cmake --build . --parallel --target  ledger-core-static
cmake --build . --parallel --target  ledger-core
cmake --build . --parallel
