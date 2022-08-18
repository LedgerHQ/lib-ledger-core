#!/usr/bin/env bash
set -e

BIN_DIR=~/lib-ledger-core-artifacts
JAR_BUILD_DIR=build-jar
JAVA_API_DIR=api/core/java
SCALA_API_DIR=api/core/scala
RESOURCE_DIR=$JAR_BUILD_DIR/src/main/resources/resources/djinni_native_libs

rm -rf $JAR_BUILD_DIR
mkdir $JAR_BUILD_DIR
mkdir -p $RESOURCE_DIR

cp $JAVA_API_DIR/* $JAR_BUILD_DIR
cp $SCALA_API_DIR/* $JAR_BUILD_DIR
cp $BIN_DIR/linux/jni/libledger-core_jni.so $RESOURCE_DIR
cp $BIN_DIR/macos/jni/libledger-core_jni.dylib $RESOURCE_DIR
cp .circleci/build.sbt $JAR_BUILD_DIR

cd $JAR_BUILD_DIR
if [ -n "$CIRCLE_TAG" ] || [ "$CIRCLE_BRANCH" == "main" -o "$CIRCLE_BRANCH" == "develop" ] || [[ "$CIRCLE_BRANCH" == "release/"* ]] ; then
	if [[ $LIB_VERSION == *"-rc-"* ]]; then
		export JAR_VERSION="$LIB_VERSION"-SNAPSHOT
	else
		export JAR_VERSION=$LIB_VERSION
	fi
  JAR_VERSION=$JAR_VERSION sbt package
  cp ./target/scala-2.12/scala-lib-core_2.12-${JAR_VERSION}.jar ~/lib-ledger-core-artifacts/ledger-lib-core.jar
fi

