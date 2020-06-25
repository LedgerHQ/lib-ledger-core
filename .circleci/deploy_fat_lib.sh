#!/usr/bin/env bash

cd ../lib-ledger-core-artifacts/ios

echo "======> Build Fat Library"
pwd

lipo -create armv7/ledger-core.framework/ledger-core arm64/ledger-core.framework/ledger-core x86_64/ledger-core.framework/ledger-core -o ledger-core
mkdir ledger-core.framework
mv ledger-core ledger-core.framework/
cp armv7/ledger-core.framework/Info.plist ledger-core.framework/
install_name_tool -add_rpath "@executable_path/Frameworks/universal" ledger-core.framework/ledger-core
lipo -info ledger-core.framework/ledger-core

if [ -n "$CIRCLE_TAG" ] || [ "$CIRCLE_BRANCH" == "master" -o "$CIRCLE_BRANCH" == "develop" ]; then
	aws s3 sync ./ s3://ledger-lib-ledger-core/$LIB_VERSION/ios/universal --acl public-read --exclude "x86_64/*" --exclude "armv7/*" --exclude "arm64/*" && \
	aws s3 ls s3://ledger-lib-ledger-core/$LIB_VERSION/ios/universal;
elif [[ "$CIRCLE_BRANCH" =~ ^int-.* ]]; then
    if [ -z "$1" ]
	then
		aws s3 sync ./ s3://ledger-lib-ledger-core/$CIRCLE_BRANCH/release/$LIB_VERSION/ios/universal --acl public-read --exclude "x86_64/*" --exclude "armv7/*" --exclude "arm64/*" && \
		aws s3 ls s3://ledger-lib-ledger-core/$CIRCLE_BRANCH/release/$LIB_VERSION/ios/universal
	else
		aws s3 sync ./ s3://ledger-lib-ledger-core/$CIRCLE_BRANCH/$1/$LIB_VERSION/ios/universal --acl public-read --exclude "x86_64/*" --exclude "armv7/*" --exclude "arm64/*" && \
		aws s3 ls s3://ledger-lib-ledger-core/$CIRCLE_BRANCH/$1/$LIB_VERSION/ios/universal
	fi
fi
