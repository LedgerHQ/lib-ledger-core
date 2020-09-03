#!/usr/bin/env bash
if [ -n "$CIRCLE_TAG" ] || [ "$CIRCLE_BRANCH" == "master" -o "$CIRCLE_BRANCH" == "develop" ] || [ "$CIRCLE_BRANCH" == "live-prerelease" ]; then
	cd ../lib-ledger-core-artifacts
	ls -la
	aws s3 sync ./ s3://ledger-lib-ledger-core/$LIB_VERSION/ --acl public-read && \
	aws s3 ls s3://ledger-lib-ledger-core/$LIB_VERSION;
	cd -
fi
