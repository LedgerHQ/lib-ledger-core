#!/usr/bin/env bash

PACKAGE_NAME=ledgercore

DEST=../lib-ledger-core-react-native-bindings
DEST_IOS=$DEST/ios/Sources/react-native-ios
DEST_IOS_OBJC=$DEST/ios/Sources/objc
DEST_IOS_OBJCPP=$DEST/ios/Sources/objcpp
DEST_ANDROID=$DEST/android/src/main/java/com/ledger/reactnative
DEST_ANDROID_JAVA_IFACE=$DEST/android/src/main/java/co/ledger/core
CORE_CPP_API=$DEST/ios/Sources/include

# prune export directories
rm -r $DEST_IOS
rm -r $DEST_IOS_OBJC
rm -r $DEST_IOS_OBJCPP
rm -r $CORE_CPP_API

./djinni/src/run \
    --idl ./core/core.djinni \
    --cpp-out $CORE_CPP_API \
    --cpp-namespace ledger::core::api \
    --cpp-optional-template std::experimental::optional \
    --cpp-optional-header "\"../utils/optional.hpp\"" \
    --objc-type-prefix LG \
    --objc-out $DEST_IOS_OBJC \
    --objcpp-out $DEST_IOS_OBJCPP \
    --react-native-objc-out $DEST_IOS \
    --react-native-type-prefix RCTCore \
    --react-include-objc-impl ../objc-impl \
    --react-native-objc-impl-suffix Impl \
    --react-native-java-out $DEST_ANDROID \
    --react-native-java-package com.ledger.reactnative \
    --java-out $DEST_ANDROID_JAVA_IFACE \
    --java-package co.ledger.core \
    --trace true
