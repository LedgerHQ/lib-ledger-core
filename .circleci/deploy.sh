#!/usr/bin/env bash

unamestr=`uname`
BUILD_CONFIG=$1

branchstr=`git rev-parse --abbrev-ref HEAD`
if [ "$branchstr" == "develop" ]; then
	echo "======> Start deploy for $unamestr"
	if [ "$unamestr" == "Linux" ]; then
		export BUILD_TYPE=linux
	elif [ "$unamestr" == "Darwin" ]; then
		export BUILD_TYPE=macos
	fi
	aws s3 sync ../lib-ledger-core-build/core/src/ s3://ledger-lib-ledger-core/latest/$BUILD_TYPE --acl public-read --exclude "CMakeFiles/*" --exclude "Makefile" --exclude "cmake_install.cmake" && \
	aws s3 ls s3://ledger-lib-ledger-core/latest/$BUILD_TYPE;
else
	echo "======> Deployment only on Develop (temporary) branch"
fi
