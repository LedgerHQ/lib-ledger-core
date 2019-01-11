#!/usr/bin/env bash
echo "=====>Install nvm"
curl -o- https://raw.githubusercontent.com/creationix/nvm/v0.33.11/install.sh | bash
export NVM_DIR="${XDG_CONFIG_HOME/:-$HOME/.}nvm"
[ -s "$NVM_DIR/nvm.sh" ] && \. "$NVM_DIR/nvm.sh"
source ~/.bashrc

echo "=====>Change node version"
node --version
nvm install 8.9.4
nvm use 8.9.4
node --version

echo "=====>Build node module"

cd ledger-core-samples/nodejs
yarn
mkdir tmp
node tests/basic-test.js

LIB_VERSION=$(node tests/lib-version.js)

if [ -z "$CIRCLE_TAG" ]; then
	LIB_VERSION="$LIB_VERSION-rc"
fi

echo "export LIB_VERSION=$LIB_VERSION" >> $BASH_ENV

echo "=====> Libcore version"
echo $LIB_VERSION