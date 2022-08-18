#!/usr/bin/env bash
set -e

BUILD_TYPE=$1

if [ ! -d lib-ledger-core-build-env ]
then
  git clone git@github.com:LedgerHQ/lib-ledger-core-build-env.git
else
  cd lib-ledger-core-build-env
  git pull
  cd ..
fi

cd lib-ledger-core-build-env
docker build -t lib-ledger-core-build-env:latest .
cd ..

mkdir -p artifacts
docker run \
    --mount type=bind,source="$(pwd)"/artifacts,target=/hostdir \
    --mount type=bind,source="$(pwd)"/tools,target=/root/tools \
    --mount type=bind,source="$(pwd)",target=/root/lib-ledger-core-ro,readonly \
    lib-ledger-core-build-env:latest \
    /root/tools/prod-like-build-inner.sh ${BUILD_TYPE}

