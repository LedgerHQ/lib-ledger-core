#!/usr/bin/env bash

unamestr=$(uname)
branchstr=$(git branch | grep '*' | sed 's/^..//')

cd /Users/distiller/ios

echo "======> Build Fat Library"
pwd
lipo -create x86_64/libledger-core.dylib armv7/libledger-core.dylib arm64/libledger-core.dylib -o libledger-core.dylib
lipo -info libledger-core.dylib

echo "======> Deploy from $branchstr branch, version : $LIB_VERSION"

if [ "$branchstr" == "develop" ]; then
	echo "======> Start deploy fat library for $unamestr"
	aws s3 sync ./ s3://ledger-lib-ledger-core/$LIB_VERSION/ios/universal --acl public-read --exclude "x86_64/*" --exclude "armv7/*" --exclude "arm64/*" && \
	aws s3 ls s3://ledger-lib-ledger-core/$LIB_VERSION/ios/universal;

else
	echo "======> Deployment only on Develop (temporary) branch"
fi
