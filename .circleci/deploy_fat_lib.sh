#!/usr/bin/env bash

unamestr=$(uname)
branchstr=$(git branch | grep '*' | sed 's/^..//')

cd /Users/distiller/ios

echo "======> Build Fat Library"
pwd

#We don't put x86_64/ledger-core.framework/ledger-core otherwise we have a problem when pushing to AppStore
lipo -create armv7/ledger-core.framework/ledger-core arm64/ledger-core.framework/ledger-core -o ledger-core
mkdir ledger-core.framework
mv ledger-core ledger-core.framework/
cp arm64/ledger-core.framework/Info.plist ledger-core.framework/
lipo -info ledger-core.framework/ledger-core

echo "======> Deploy from $branchstr branch, version : $LIB_VERSION"

if [ "$branchstr" == "develop" ]; then
	echo "======> Start deploy fat library for $unamestr"
	aws s3 sync ./ s3://ledger-lib-ledger-core/$LIB_VERSION/ios/universal --acl public-read --exclude "x86_64/*" --exclude "armv7/*" --exclude "arm64/*" && \
	aws s3 ls s3://ledger-lib-ledger-core/$LIB_VERSION/ios/universal;

else
	echo "======> Deployment only on Develop (temporary) branch"
fi
