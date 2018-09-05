#!/usr/bin/env bash

echo "=====>Create build directory"
cd .. && mkdir lib-ledger-core-build && cd lib-ledger-core-build

echo "=====>Start build"
unamestr=`uname`
BUILD_CONFIG=$1
echo "======> CMake config for $unamestr in $BUILD_CONFIG mode"

if [ "$BUILD_CONFIG" == "Release" ]; then
    cmake -DBUILD_TESTS=OFF ../lib-ledger-core
else
	if [ "$unamestr" == "Linux" ]; then
		cmake -DCMAKE_INSTALL_PREFIX=$HOME ../lib-ledger-core
	elif [ "$unamestr" == "Darwin" ]; then
		version=`ls /usr/local/Cellar/qt | grep 5.`
		echo "====> Get qt5 version"
		echo $version
		export PATH="/usr/local/Cellar/qt/$version/bin:$PATH"
		echo $PATH
	   	cmake -DCMAKE_INSTALL_PREFIX="/usr/local/Cellar/qt/$version" -DCMAKE_PREFIX_PATH="/usr/local/Cellar/qt/$version" ../lib-ledger-core
	fi
fi

echo "======> Build for $unamestr in $BUILD_CONFIG mode"
make -j4

