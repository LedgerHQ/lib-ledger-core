#!/usr/bin/env bash

unamestr=$(uname)
TARGET=$1
ARCH=$2
branchstr=$(git branch | grep '*' | sed 's/^..//')

echo "======> Start deploy for $unamestr"
if [ "$unamestr" == "Linux" ]; then
	if [ "$TARGET" == "target_jni" ]; then
		mv ../lib-ledger-core-build/core/src/libledger-core.so ../lib-ledger-core-build/core/src/libledger-core_jni.so
		export BUILD_TYPE=linux/jni
	elif [ "$TARGET" == "android" ]; then
		export BUILD_TYPE=android/${ARCH}
	else
		export BUILD_TYPE=linux
	fi
elif [ "$unamestr" == "Darwin" ]; then
	if [ "$TARGET" == "target_jni" ]; then
		mv ../lib-ledger-core-build/core/src/libledger-core.dylib ../lib-ledger-core-build/core/src/libledger-core_jni.dylib
		export BUILD_TYPE=macos/jni
	else
		export BUILD_TYPE=macos
	fi
fi

PATH_TO_LIB=../lib-ledger-core-build/core/src/
if [ "$TARGET" == "ios" ]; then
	if [ "$ARCH" == "armv7" -o "$ARCH" == "arm64" ]; then
		export BUILD_TYPE=ios/${ARCH}
		PATH_TO_LIB=../lib-ledger-core-build/core/src/Release-iphoneos
	else
		export BUILD_TYPE=ios/x86_64
		PATH_TO_LIB=../lib-ledger-core-build/core/src/Release-iphonesimulator
	fi
elif [ "$TARGET" == "android" ]; then
	if [ "$ARCH" == "armeabi-v7a" -o "$ARCH" == "arm64-v8a" -o "$ARCH" == "x86_64" ]; then
		export BUILD_TYPE=android/${ARCH}
	else
		export BUILD_TYPE=android/x86
	fi
fi

echo "======> Copy artifact to lib-ledger-core-artifacts"
mkdir -p ../lib-ledger-core-artifacts/$BUILD_TYPE
cp -r $PATH_TO_LIB/*ledger-core* ../lib-ledger-core-artifacts/$BUILD_TYPE

