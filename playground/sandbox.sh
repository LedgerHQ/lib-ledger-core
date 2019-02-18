#!/bin/zsh

mkdir -p /tmp/lib-core-sandbox
tmp="/tmp/lib-core-sandbox"
libcore="lib-ledger-core"
libcore_dir="$(pwd)/$1"
libcore_dylib="libledger-core.dylib"
node_binding="lib-ledger-core-node-bindings"
node_binding_dir="$tmp/$node_binding"
live="ledger-live-desktop"
live_dir="$tmp/$live"

function title {
    echo -e "\e[37m┌ \e[1m\e[34m$*\e[0m"
}

function say {
    echo -e "\e[37m├ \e[1m\e[92m$*\e[0m"
}

# load nvm if we have the script available
[ -s "/usr/local/opt/nvm/nvm.sh" ] && . "/usr/local/opt/nvm/nvm.sh"

cd $tmp
echo "Working directory is $tmp"

title "Cloning repositories"
say "Cloning node bindings"
git clone --depth 1 https://github.com/LedgerHQ/$node_binding $node_binding_dir &> /dev/null

say "Cloning Live Desktop"
git clone --depth 1 https://github.com/LedgerHQ/$live $live_dir &> /dev/null

title "Compiling libcore"
mkdir -p $libcore_dir/build
cd $libcore_dir/build

say "Configuring"
cmake -DCMAKE_INSTALL_PREFIX=/usr/local/Cellar/qt/5.11.2_1 -DCMAKE_BUILD_TYPE=Debug .. &> $tmp/libcore_config.output

say "Compiling"
make -j8 &> $tmp/libcore_make.output

title "Patching node bindings"
cd $node_binding_dir
say "Removing AWS S3 preinstall"
sed -i "" -E /"preinstall"/d package.json

say "Pinning local version"
sed -i "" -E s/'"version": "([0-9]+\.[0-9]\.[0-9]).*"'/'"version": "\1-local"'/ package.json
#node_binding_version=$(grep "version" package.json | sed -E s/'^.*"(.*-local)".*'/'\1'/)
say "Bindings will be using \e[31m$node_binding_version"

say "Linking $libcore dynamic library"
mkdir -p lib
cp $libcore_dir/build/core/src/$libcore_dylib lib/ &> /dev/null

say "Generating bindings"
cd $libcore_dir
./tools/generateBindings.sh $node_binding_dir $libcore_dir/build &> $tmp/libcore_generate_bindings.output
git checkout core/src/jni

cd $node_binding_dir
say "Creating the node module"
nvm install 8.14.0 &> $tmp/nvm_install.output
nvm use 8.14.0 &> $tmp/nvm_use.output
yarn &> $tmp/node_binding_yarn.output

title "Setting up Live Desktop"
cd $live_dir

say "Replacing the node.js binding module"
yarn add $node_binding_dir

say "Hacking around the dynamic library to enable auto-updating it from the lib core project"
rm node_modules/@ledgerhq/ledger-core/build/Release/libledger-core.dylib
cp $libcore_dir/build/core/src/$libcore_dylib node_modules/@ledgerhq/ledger-core/build/Release/

say "Building Live Desktop"
yarn &> $tmp/live_yarn.output

title "Running"
yarn start
