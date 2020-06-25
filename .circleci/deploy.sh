#!/usr/bin/env bash
if [ -n "$CIRCLE_TAG" ] || [ "$CIRCLE_BRANCH" == "master" -o "$CIRCLE_BRANCH" == "develop" ]; then
	cd ../lib-ledger-core-artifacts
	ls -la
	aws s3 sync ./ s3://ledger-lib-ledger-core/$LIB_VERSION/ --acl public-read && \
	aws s3 ls s3://ledger-lib-ledger-core/$LIB_VERSION;
	cd -
elif [[ "$CIRCLE_BRANCH" =~ ^int-.* ]]; then
	cd ../lib-ledger-core-artifacts
	ls -la
    if [ -z "$1" ]
	then
    	aws s3 sync ./ s3://ledger-lib-ledger-core/$CIRCLE_BRANCH/release/$LIB_VERSION --acl public-read && \
		aws s3 ls s3://ledger-lib-ledger-core/$CIRCLE_BRANCH/release/$LIB_VERSION;
	else
		aws s3 sync ./ s3://ledger-lib-ledger-core/$CIRCLE_BRANCH/$1/$LIB_VERSION --acl public-read && \
		aws s3 ls s3://ledger-lib-ledger-core/$CIRCLE_BRANCH/$1/$LIB_VERSION;
	fi
	cd -
fi
