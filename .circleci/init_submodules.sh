#!/usr/bin/env bash

echo "========> Add github as known host"
[ -d "${HOME}/.ssh" ] || mkdir "${HOME}/.ssh"
ssh-keyscan github.com >> "${HOME}/.ssh/known_hosts"

cd $HOME/lib-ledger-core

git submodule init
git submodule sync

#Otherwise can't init submodules
echo "========> Update all submodules "
#No need for those
git submodule update -- djinni || echo "===========Djinni submodule already updated"
git submodule update -- toolchains/polly || echo "===========Polly submodule already updated"
#git submodule update -- tools/gyp || echo "===========gyp submodule already updated"
git submodule update -- core/lib/spdlog || echo "===========spdlog submodule already updated"
git submodule update -- core/lib/leveldb || echo "===========leveldb submodule already updated"
git submodule update -- core/lib/ethash || echo "===========ethash submodule already updated"
#should checkout leveldb bitcoin-fork branch on leveldb submodule
cd $HOME/lib-ledger-core/core/lib/leveldb && git checkout bitcoin-fork

echo "========> Generate ITFs"
cd $HOME/lib-ledger-core
./tools/generate_interfaces.sh

unamestr=`uname`
#This is a workaround because building directly
#SQLCipher for arm64 and armv7 is failing
#for more details please refer to comments
#in core/lib/cmake/configure_sqlcipher_ios.sh
if [ "$unamestr" == "Darwin" ]; then
	echo "========> Generating SQLCipher for iOS"
	./build_sqlcipher_ios_arm64_armv7.sh armv7
	./build_sqlcipher_ios_arm64_armv7.sh arm64
fi
