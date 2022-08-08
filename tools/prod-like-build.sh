#!/usr/bin/env bash
set -e

if [ ! -d lib-ledger-core-build-env ]
then
  git clone git@github.com:LedgerHQ/lib-ledger-core-build-env.git
else
  cd lib-ledger-core-build-env
  git pull
  cd ..
fi
cp tools/prod-like-build-inner.sh lib-ledger-core-build-env/scripts/
cd lib-ledger-core-build-env
git checkout update-installation
docker build -t lib-ledger-core-build-env:latest .
cd ..
mkdir -p artifacts
docker run \
    --mount type=bind,source="$(pwd)"/artifacts,target=/hostdir \
    lib-ledger-core-build-env:latest \
    /scripts/prod-like-build-inner.sh Debug

