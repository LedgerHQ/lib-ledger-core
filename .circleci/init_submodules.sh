#!/usr/bin/env bash

echo "========> Add github as known host"
[ -d "${HOME}/.ssh" ] || mkdir "${HOME}/.ssh"
ssh-keyscan github.com >> "${HOME}/.ssh/known_hosts"

cd $HOME/lib-ledger-core

git submodule init
git submodule sync

#Otherwise can't init submodules
echo "========> Update all submodules "
git submodule update

#should checkout leveldb bitcoin-fork branch on leveldb submodule
cd $HOME/lib-ledger-core/core/lib/leveldb && git checkout bitcoin-fork
