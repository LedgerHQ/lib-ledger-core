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
LIBCORE_GIT_DESCRIBE=`git describe --tags --first-parent`

echo "=====> Libcore version : $LIBCORE_VERSION"
echo "GITHUB_BASE_REF: ${GITHUB_BASE_REF}"

# https://docs.github.com/en/actions/reference/workflow-commands-for-github-actions#setting-an-output-parameter
echo "::set-output name=lib_version::$LIBCORE_VERSION"

printf "# github event is **${GITHUB_EVENT_NAME}**\n"
if [[ "${GITHUB_EVENT_NAME}" == "release" ]]; then
    printf "\nMarking for deploy\n"
    echo "::set-output name=deploy_dynlibs::YES"
    echo "::set-output name=jar_version::${LIB_VERSION}"
elif [[ "${GITHUB_EVENT_NAME}" == "push" ]]; then
    printf "\nMarking for snapshot\n"
    echo "::set-output name=deploy_dynlibs::NO"
    echo "::set-output name=jar_version::${LIB_VERSION}-${LIBCORE_GIT_DESCRIBE}-SNAPSHOT"
elif [[ "${GITHUB_EVENT_NAME}" == "pull_request" ]]; then
    printf "\nMarking for snapshot\n"
    BRANCH_NAME=$1
    BRANCH_LENGTH=`git rev-list --count ^remotes/origin/${GITHUB_BASE_REF} remotes/origin/${BRANCH_NAME}`
    ABBREV_COMMIT_HASH=`git rev-list ^remotes/origin/${GITHUB_BASE_REF} remotes/origin/${BRANCH_NAME} | head -n 1 | cut -c 1-6`
    echo "::set-output name=deploy_dynlibs::NO"
    echo "::set-output name=jar_version::${LIB_VERSION}-${BRANCH_NAME}-${BRANCH_LENGTH}-${ABBREV_COMMIT_HASH}-SNAPSHOT"
fi

