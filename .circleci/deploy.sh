#!/usr/bin/env bash

unamestr=$(uname)
TARGET=$1
#branchstr=`git rev-parse --abbrev-ref HEAD`
branchstr=$(git branch | grep '*' | sed 's/^..//')

echo "======> Deploy from $branchstr branch, version : $LIB_VERSION"

if [ "$branchstr" == "develop" -o "$branchstr" == "jni-ci" ]; then
	echo "======> Start deploy for $unamestr"
	if [ "$unamestr" == "Linux" ]; then
		if [ "$TARGET" == "target_jni" ]; then
			mv ../lib-ledger-core-build/core/src/libledger-core.so ../lib-ledger-core-build/core/src/libledger-core_jni.so
			export BUILD_TYPE=linux/jni
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

	echo "======> Deploy for $unamestr"
	aws s3 sync ../lib-ledger-core-build/core/src/ s3://ledger-lib-ledger-core/$LIB_VERSION/$BUILD_TYPE --acl public-read --exclude "CMakeFiles/*" --exclude "Makefile" --exclude "cmake_install.cmake" && \
	aws s3 ls s3://ledger-lib-ledger-core/$LIB_VERSION/$BUILD_TYPE;

else
	echo "======> Deployment only on Develop (temporary) branch"
fi
