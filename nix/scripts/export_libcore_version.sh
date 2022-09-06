#!/usr/bin/env bash
set -euo pipefail

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
    LIBCORE_VERSION=${LIB_VERSION}-${BRANCH_NAME}
    JAR_VERSION="${LIBCORE_VERSION}-SNAPSHOT"
fi

echo "::set-output name=jar_version::${JAR_VERSION}"
echo "::set-output name=lib_version::${LIBCORE_VERSION}"
echo "=====> Libcore version : $LIBCORE_VERSION"
echo "=====> JAR version : $JAR_VERSION"
