#!/usr/bin/env bash

unamestr=$(uname)
TARGET=$1
ARCH=$2
#branchstr=`git rev-parse --abbrev-ref HEAD`
branchstr=$(git branch | grep '*' | sed 's/^..//')

echo "======> Deploy from $branchstr branch, version : $LIB_VERSION"

if [ "$branchstr" == "develop" ]; then
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
		PATH_TO_LIB=../lib-ledger-core-build/core/src/Release-iphoneos
		if [ "$ARCH" == "armv7" -o "$ARCH" == "arm64" ]; then
			export BUILD_TYPE=ios/${ARCH}
		else
			export BUILD_TYPE=ios/x86_64
		fi
	elif [ "$TARGET" == "android" ]; then
		if [ "$ARCH" == "armeabi-v7a" -o "$ARCH" == "arm64-v8a" ]; then
			export BUILD_TYPE=android/${ARCH}
		else
			export BUILD_TYPE=android/x86
		fi
	fi
	echo "======> Deploy for $unamestr"
	aws s3 sync $PATH_TO_LIB s3://ledger-lib-ledger-core/$LIB_VERSION/$BUILD_TYPE --acl public-read --exclude "CMakeFiles/*" --exclude "Makefile" --exclude "cmake_install.cmake" && \
	aws s3 ls s3://ledger-lib-ledger-core/$LIB_VERSION/$BUILD_TYPE;

else
	echo "======> Deployment only on Develop (temporary) branch"
fi
