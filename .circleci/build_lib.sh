#!/usr/bin/env bash

echo "=====>Create build directory"
cd .. && mkdir lib-ledger-core-build && cd lib-ledger-core-build

echo "=====>Start build"
unamestr=`uname`

if [[ "$unamestr" == 'Linux' ]]; then
	echo "======> Build for Linux"
   cmake -DCMAKE_INSTALL_PREFIX=$HOME ../lib-ledger-core
elif [[ "$unamestr" == 'Darwin' ]]; then
	echo "======> Build for macOS"
	version=`ls /usr/local/Cellar/qt | grep 5.`
	echo "====> Get qt5 version"
	echo $version
	export PATH="/usr/local/Cellar/qt/$version/bin:$PATH"
	echo $PATH
   cmake -DCMAKE_INSTALL_PREFIX="/usr/local/Cellar/qt/$version" -DCMAKE_PREFIX_PATH="/usr/local/Cellar/qt/$version" ../lib-ledger-core
fi

make -j4

