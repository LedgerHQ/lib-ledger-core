#!/bin/bash
set -euo pipefail

IFS=. read major minor patch <<<$(java -version 2>&1 | awk -F '"' '/version/ {print $2}')
if [ $major -ne 1 ] | [ $minor -ne 8 ]; then
    printf "Openjdk should match version 1.8\n\n"
    printf "Tip ! you can use:\n"
    printf "$ jabba use adopt@1.8.0-292\n"
    exit 1
fi

# This script works with a directory name as first parameter
# You need to have the libcore already built in this directory with JNI enabled
# e.g.:
# cd lib_build_dir_jni
# cmake .. -DSYS_OPENSSL=ON -DOPENSSL_USE_STATIC_LIBS=TRUE -DTARGET_JNI=ON -DPG_SUPPORT=ON
# cmake --build . --parallel
#
# And then you call the script:
# ./publish-jar-local.sh ./lib_build_dir_jni
LIB_BUILD_DIR=$1

JAR_BUILD_DIR=/tmp/build-jar
JAVA_API_DIR=api/core/java
SCALA_API_DIR=api/core/scala
SBT_DIR=nix/scripts/sbt
RESOURCE_DIR=$JAR_BUILD_DIR/src/main/resources/resources/djinni_native_libs

rm -rf $JAR_BUILD_DIR
mkdir -v $JAR_BUILD_DIR

mkdir -v -p $RESOURCE_DIR

cp $JAVA_API_DIR/* $JAR_BUILD_DIR
cp $SCALA_API_DIR/* $JAR_BUILD_DIR
cp -r $SBT_DIR/* $JAR_BUILD_DIR

cp $LIB_BUILD_DIR/core/src/libledger-core.so $RESOURCE_DIR

cd $JAR_BUILD_DIR
export GITHUB_TOKEN="unused" # but mandatory

# By default it will generate a package named local-SNAPSHOT, but you can override it by giving a 2nd parameter to the script
# e.g.  ./publish-jar-local.sh ./lib_build_dir_jni custom_name
#         -> will generate package custom_name-SNAPSHOT
# You may find it under ~/.ivy2/local/co.ledger/ledger-lib-core_2.12/local-SNAPSHOT
export JAR_VERSION=${2-local}-SNAPSHOT

sbt publishLocal
