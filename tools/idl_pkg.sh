#!/usr/bin/env bash
#
# Generate library bindings with djinni.
# Invocate with TRACE=1 as environment variable to set debug compilation.

set -e

usage() {
  echo -e $1
  exit $2
}

if [[ $TRACE ]] ; then
  echo "Debug compilation enabled"
  trace="true";
else
  trace="false";
fi

function generate_npm_interface {
  NODE_DIR=./bindings/node
  NODE_SRC_DIR=$NODE_DIR/src

  # recreate node directory
  rm -rf $NODE_SRC_DIR
  mkdir -p $NODE_SRC_DIR
  rm -rf $NODE_DIR/lib

  rm -rf $NODE_DIR/include
  mkdir -p $NODE_DIR/include
  rm -rf $NODE_DIR/include/utils
  mkdir -p $NODE_DIR/include/utils

  # create tmp folder if needed
  if [ ! -e $NODE_DIR/js/tmp ]; then
    mkdir -p $NODE_DIR/js/tmp
  fi

  API_DIR=./bundle/api
  BUILD_DIR=${BUILD_DIR:-build}
  NODE_PKG_NAME=ledgercore

  echo -e "Generating Node binding code"

  ./djinni/src/run \
    --idl ./bundle/bundle.djinni \
    --cpp-out $API_DIR \
    --cpp-namespace ledger::core::api \
    --cpp-optional-template std::experimental::optional \
    --cpp-optional-header "\"utils/Optional.hpp\"" \
    --node-out $NODE_SRC_DIR \
    --node-type-prefix NJS \
    --node-include-cpp "../include" \
    --node-package $NODE_PKG_NAME \
    --export-header-name libcore_export \
    --trace $trace

  # copy include files
  echo "Copying header files…"
  cp -r $API_DIR/* $NODE_DIR/include

  # copy util files
  echo "Copying utils files…"
  cp -r ledger-core/inc/core/utils/Optional.hpp $NODE_DIR/include/utils
  cp -r ledger-core/inc/core/LibCoreExport.hpp $NODE_DIR/include

  # copy lib files
  echo "Copying lib files"
  mkdir -p $NODE_DIR/lib

  # copy dynamic library
  echo "Copying the dynamic library"
  cp $BUILD_DIR/libledger-core-bundle.* $NODE_DIR/lib

  # generate the NPM package
  echo "Generating the NPM package"
  cd $NODE_DIR
  yarn

  # (optional) if yalc is found, publish and push the package to the local store
  if yalc --version &> /dev/null; then
    echo "Publishing and pushing to the local yalc store"
    yalc publish --push
  fi
}

function generate_react_native_interface {
  RN_DIR=./bindings/react-native
  RN_IOS=$RN_DIR/ios/Sources/react-native-ios
  RN_IOS_OBJC=$RN_DIR/ios/Sources/objc
  RN_IOS_OBJCPP=$RN_DIR/ios/Sources/objcpp
  RN_IOS_SOURCES=$RN_DIR/ios/Sources/include
  RN_ANDROID_LIBS=$RN_DIR/android/libs
  RN_ANDROID_JAVA_OUT=$RN_DIR/android/src/main/java/com/ledger/reactnative
  RN_ANDROID_JAVA_IFACE=$RN_DIR/android/src/main/java/co/ledger/core
  API_DIR=./bundle/api
  BUILD_DIR=${BUILD_DIR:-build}

  # prune export directories
  rm -rf $RN_IOS
  rm -rf $RN_IOS_OBJC
  rm -rf $RN_IOS_OBJCPP
  rm -rf $RN_IOS_SOURCES

  rm -rf $RN_ANDROID_LIBS

  for coin in $*; do
    if [ "$coin" == "core" ]; then
      PROJECT_NAME=ledger-core
      RN_PKG_NAME=ledgercore
    else
      PROJECT_NAME=ledger-core-$coin
      RN_PKG_NAME=ledgercore-$coin
    fi

    CURRENCY_NAME=$coin

    echo -e "Generating $PROJECT_NAME ($CURRENCY_NAME) React Native binding code"

    ./djinni/src/run \
      --idl $PROJECT_NAME/idl/idl.djinni \
      --cpp-out $RN_IOS_SOURCES \
      --cpp-namespace ledger::core::api \
      --cpp-optional-template std::experimental::optional \
      --cpp-optional-header "\"../utils/optional.hpp\"" \
      --objc-type-prefix LG \
      --objc-out $RN_IOS_OBJC \
      --objcpp-out $RN_IOS_OBJCPP \
      --react-native-objc-out $RN_IOS \
      --react-native-type-prefix RCTCore \
      --react-include-objc-impl ../objc-impl \
      --react-native-objc-impl-suffix Impl \
      --react-native-java-out $RN_ANDROID_JAVA_OUT \
      --react-native-java-package com.ledger.reactnative \
      --java-out $RN_ANDROID_JAVA_IFACE \
      --java-package co.ledger.core \
      --trace $trace
  done

  # copy dynamic libraries
  CORE_LIB_x86=${BUILD_DIR}_x86/src/libledger-core.so
  if [[ -f $CORE_LIB_x86 ]]; then
    echo "Copying the dynamic library (x86) for Android"
    cp $CORE_LIB_x86 $RN_ANDROID_LIBS/x86
  fi

  CORE_LIB_x86_64=${BUILD_DIR}_x86_64/src/libledger-core.so
  if [[ -f $CORE_LIB_x86_64 ]]; then
    echo "Copying the dynamic library (x86_64) for Android"
    cp $CORE_LIB_x86_64 $RN_ANDROID_LIBS/x86_64
  fi

  CORE_LIB_armeabi_v7a=${BUILD_DIR}_armeabi-v7a/src/libledger-core.so
  if [[ -f $CORE_LIB_armeabi_v7a ]]; then
    echo "Copying the dynamic library (armeabi-v7a) for Android"
    cp $CORE_LIB_armeabi_v7a $RN_ANDROID_LIBS/armeabi-v7a
  fi

  CORE_LIB_arm64_v8a=${BUILD_DIR}_arm64-v8a/src/libledger-core.so
  if [[ -f $CORE_LIB_arm64_v8a ]]; then
    echo "Copying the dynamic library (arm64-v8a) for Android"
    cp $CORE_LIB_arm64_v8a $RN_ANDROID_LIBS/arm64-v8a
  fi
}

function generate_scala_interface {
  JAR_BUILD_DIR=./bindings/scala
  BUNDLE_BUILD=./build_jni
  JAVA_API_DIR=./bundle/java
  SCALA_API_DIR=./bundle/scala
  RESOURCE_DIR=$JAR_BUILD_DIR/src/main/resources/resources/djinni_native_libs

  # Clean up any previous build.
  rm -rf $JAR_BUILD_DIR
  mkdir -p $JAR_BUILD_DIR
  mkdir -p $RESOURCE_DIR

  # Copy all the artifacts (Java sources, Scala package metadata and libs).
  cp -v $JAVA_API_DIR/* $JAR_BUILD_DIR
  cp -v $SCALA_API_DIR/* $JAR_BUILD_DIR
  cp -v $BUNDLE_BUILD/libledger-core-bundle.* $RESOURCE_DIR

  # Generate the Jean-Michel JAR.
  cd $JAR_BUILD_DIR
  sbt package
}

case "$1" in
  "npm")
    generate_npm_interface
    ;;

  "rn")
    generate_react_native_interface
    ;;

  "scala")
    generate_scala_interface
    ;;

  *)
    echo "unknown packaging system"
    exit 1
    ;;
esac
