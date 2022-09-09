#!/usr/bin/env bash
set -euo

function match {
    sed -nr $*
}


LIB_VERSION_MAJOR=`cat CMakeLists.txt | match 's/set\(VERSION_MAJOR.*([0-9]+).*\)/\1/p'`
LIB_VERSION_MINOR=`cat CMakeLists.txt | match 's/set\(VERSION_MINOR.*([0-9]+).*\)/\1/p'`
LIB_VERSION_PATCH=`cat CMakeLists.txt | match 's/set\(VERSION_PATCH.*([0-9]+).*\)/\1/p'`
LIB_VERSION=$LIB_VERSION_MAJOR.$LIB_VERSION_MINOR.$LIB_VERSION_PATCH

printf "# github event is **${GITHUB_EVENT_NAME}**\n"
if [[ "${RELEASE}" == "true" ]]; then
    printf "\nMarking for deploy\n"
    echo "::set-output name=deploy_dynlibs::YES"
    if [[ "${VERSION:=none}" != "none" ]]; then
      LIBCORE_VERSION=${VERSION}
    else
      LIBCORE_VERSION=${LIB_VERSION}
    fi
    JAR_VERSION=${LIBCORE_VERSION}
else
    printf "\nMarking for snapshot\n"
    echo "::set-output name=deploy_dynlibs::NO"
    BRANCH_NAME=${GITHUB_REF_NAME}
    echo "Branch name: " $BRANCH_NAME
    echo "Base ref name: "${GITHUB_BASE_REF:=main}
    BRANCH_LENGTH=`git rev-list --count ^remotes/origin/${GITHUB_BASE_REF:=main} remotes/origin/${BRANCH_NAME}`
    echo "Branch length: " $BRANCH_LENGTH
    # syntax (git ... ||:) is to avoid SIGPIPE error: https://stackoverflow.com/a/19120674
    ABBREV_COMMIT_HASH=`(git rev-list ^remotes/origin/${GITHUB_BASE_REF:=main} remotes/origin/${BRANCH_NAME} ||:) | head -n 1 | cut -c 1-6`
    echo "Commit hash: " $ABBREV_COMMIT_HASH
    LIBCORE_VERSION=${LIB_VERSION}-${BRANCH_NAME}-${BRANCH_LENGTH}-${ABBREV_COMMIT_HASH}
    JAR_VERSION="${LIBCORE_VERSION}-SNAPSHOT"
fi

echo "::set-output name=jar_version::${JAR_VERSION}"
echo "::set-output name=lib_version::${LIBCORE_VERSION}"
echo "=====> Libcore version : $LIBCORE_VERSION"
echo "=====> JAR version : $JAR_VERSION"
