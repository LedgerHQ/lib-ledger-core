#!/usr/bin/env bash
set -euo pipefail

function match {
    sed -nr $*
}


LIB_VERSION_MAJOR=`cat CMakeLists.txt | match 's/set\(VERSION_MAJOR.*([0-9]+).*\)/\1/p'`
LIB_VERSION_MINOR=`cat CMakeLists.txt | match 's/set\(VERSION_MINOR.*([0-9]+).*\)/\1/p'`
LIB_VERSION_PATCH=`cat CMakeLists.txt | match 's/set\(VERSION_PATCH.*([0-9]+).*\)/\1/p'`
LIB_VERSION=$LIB_VERSION_MAJOR.$LIB_VERSION_MINOR.$LIB_VERSION_PATCH

COMMIT_HASH=`echo $GITHUB_SHA | cut -c 1-6`
LIB_VERSION="$LIB_VERSION-rc-$COMMIT_HASH"

echo "=====> Libcore version : $LIB_VERSION"
echo "::set-output name=lib_version::$LIB_VERSION"

if [[ "${GITHUB_EVENT_NAME:-NO}" == "push" ]]; then
    printf "\nMarking for deploy\n"
    echo "::set-output name=deploy_jar::YES"
    echo "::set-output name=deploy_dynlibs::YES"
else
    printf "\n${GITHUB_EVENT_NAME:-NO} is not \"push\", so not marking for deploy\n"
    echo "::set-output name=deploy_jar::NO"
    echo "::set-output name=deploy_dynlibs::NO"
fi
