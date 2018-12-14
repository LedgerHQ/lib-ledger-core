#!/usr/bin/env bash

PACKAGE_NAME=ledgercore
DEST=../lib-ledger-core-react-native-bindings
# CORE_BUILD=../lib-ledger-core-build

CORE_CPP_API=core/src/api

./djinni/src/run \
    --idl ./core/core.djinni \
    --cpp-out $CORE_CPP_API \
    --cpp-namespace ledger::core::api \
    --cpp-optional-template std::experimental::optional \
    --cpp-optional-header "\"../utils/optional.hpp\"" \
    --objc-type-prefix LG \
    --objc-out api/core/objc \
    --objcpp-out api/core/objcpp \
    --react-native-objc-out $DEST/ios/Sources/react-native-ios \
    --react-native-type-prefix RCTCore \
    --react-include-objc-impl  ../objc-impl \
    --react-native-objc-impl-suffix Impl \
    --react-native-java-out $DEST/android/src/main/java/com/ledger/reactnative \
    --react-native-java-package com.ledger.reactnative \
    --java-package co.ledger.core \
    --trace true

