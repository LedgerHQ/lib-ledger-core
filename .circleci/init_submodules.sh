#!/usr/bin/env bash

#!/usr/bin/env bash

echo "========> Add github as known host"
[ -d "${HOME}/.ssh" ] || mkdir "${HOME}/.ssh"
ssh-keyscan github.com >> "${HOME}/.ssh/known_hosts"

cd $HOME/lib-ledger-core

git submodule init
git submodule sync

#Otherwise can't init submodules
echo "========> Update all submodules "
git submodule update -- djinni || echo "===========Djinni submodule already updated"
git submodule update -- toolchains/polly || echo "===========Polly submodule already updated"
git submodule update -- tools/gyp || echo "===========gyp submodule already updated"
git submodule update -- core/lib/spdlog || echo "===========spdlog submodule already updated"
git submodule update -- core/lib/leveldb || echo "===========leveldb submodule already updated"

#should checkout leveldb bitcoin-fork branch
cd $HOME/lib-ledger-core/core/lib/leveldb && git checkout bitcoin-fork
