#!/usr/bin/env bash

PACKAGE_NAME=ledgercore
DEST=../lib-ledger-core-react-native-bindings
# CORE_BUILD=../lib-ledger-core-build

CORE_CPP_API=core/src/api
CORE_CPP_JNI=core/src/jni

 if [ ! -e $DEST/src/react-native ]; then
  ./djinni/src/run \
    --idl ./core/core.djinni \
    --cpp-out $CORE_CPP_API \
    --cpp-namespace ledger::core::api \
    --cpp-optional-template std::experimental::optional \
    --cpp-optional-header "\"../utils/optional.hpp\"" \
    --jni-include-cpp-prefix "../../api/" \
    --jni-out $CORE_CPP_JNI/jni \
    --objc-type-prefix LG \
    --react-native-objc-out $DEST/src/react-native \
    --react-native-type-prefix RCTCore \
    --react-include-objc-impl  ../objc-impl \
    --react-native-objc-impl-suffix Impl \
    --trace true
 fi

# copy include files
# rm -rf $DEST/include
cp -r $CORE_CPP_API $DEST/include

# copy util files
rm -rf $DEST/src/utils
mkdir $DEST/src/utils
cp -r core/src/utils/optional.hpp $DEST/src/utils

# copy lib files
rm -rf $DEST/lib
mkdir $DEST/lib

# copy dynamic library
# cp $CORE_BUILD/core/src/libledger-core.* $DEST/lib

# create tmp folder if needed
# if [ ! -e $DEST/js/tmp ]; then
#   mkdir $DEST/js/tmp
# fi
