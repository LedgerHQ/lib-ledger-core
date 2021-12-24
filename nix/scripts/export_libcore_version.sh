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
LIBCORE_VERSION="$LIB_VERSION-rc-$COMMIT_HASH"
LIBCORE_GIT_DESCRIBE=`git describe --tags`

echo "=====> Libcore version : $LIBCORE_VERSION"
#https://docs.github.com/en/actions/reference/workflow-commands-for-github-actions#setting-an-output-parameter
echo "::set-output name=lib_version::$LIBCORE_VERSION"

printf "# github event is **${GITHUB_EVENT_NAME}**\n"
if [[ "${GITHUB_EVENT_NAME:-NO}" == "release" ]]; then
    printf "\nMarking for deploy\n"
    echo "::set-output name=deploy_dynlibs::YES"
    echo "::set-output name=jar_version::${LIB_VERSION}"
elif [[ "${GITHUB_EVENT_NAME:-NO}" == "push" ]]; then
    printf "\n${GITHUB_EVENT_NAME:-NO} is \"push\", creating a snapshot\n"
    echo "::set-output name=deploy_dynlibs::NO"
    echo "::set-output name=jar_version::${LIB_VERSION}-${LIBCORE_GIT_DESCRIBE}-SNAPSHOT"
elif [[ "${GITHUB_EVENT_NAME:-NO}" == "pull_request" ]]; then
    printf "\n${GITHUB_EVENT_NAME:-NO} is \"pull_request\", creating a snapshot\n"
    echo "::set-output name=deploy_dynlibs::NO"
    echo "::set-output name=jar_version::${LIB_VERSION}-${LIBCORE_GIT_DESCRIBE}-SNAPSHOT"
fi
