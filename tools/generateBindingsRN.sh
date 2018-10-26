#!/usr/bin/env bash

PACKAGE_NAME=ledgercore
DEST=../lib-ledger-core-react-native-bindings
# CORE_BUILD=../lib-ledger-core-build

CORE_CPP_API=core/src/api

 if [ ! -e $DEST/ios/Sources/react-native-ios ]; then
  ./djinni/src/run \
    --idl ./core/core.djinni \
    --cpp-out $CORE_CPP_API \
    --cpp-namespace ledger::core::api \
    --cpp-optional-template std::experimental::optional \
    --cpp-optional-header "\"../utils/optional.hpp\"" \
    --objc-type-prefix LG \
    --react-native-objc-out $DEST/ios/Sources/react-native-ios \
    --react-native-type-prefix RCTCore \
    --react-include-objc-impl  ../objc-impl \
    --react-native-objc-impl-suffix Impl \
    --trace true
 fi

