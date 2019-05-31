#!/usr/bin/env bash
unamestr=$(uname)

if [ "$unamestr" == "Linux" ]; then
	BIN=libledger-core_jni.so
elif [ "$unamestr" == "Darwin" ]; then
	BIN=libledger-core_jni.dylib
fi
BIN_DIR=../lib-ledger-core-build/core/src
JAR_BUILD_DIR=build-jar
JAVA_API_DIR=api/core/java
SCALA_API_DIR=api/core/scala
RESOURCE_DIR=$JAR_BUILD_DIR/src/main/resources/resources/djinni_native_libs

rm -rf $JAR_BUILD_DIR
mkdir $JAR_BUILD_DIR
mkdir -p $RESOURCE_DIR

cp $JAVA_API_DIR/* $JAR_BUILD_DIR
cp $SCALA_API_DIR/* $JAR_BUILD_DIR
cp $BIN_DIR/$BIN $RESOURCE_DIR
cp .circleci/build.sbt $JAR_BUILD_DIR

