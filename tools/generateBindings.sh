#!/usr/bin/env bash

PACKAGE_NAME=ledger-core
DEST=../lib-ledger-core-node-bindings
CORE_BUILD=../lib-ledger-core-build

CORE_CPP_API=core/src/api
CORE_CPP_JNI=core/src/jni

if [ ! -e $DEST/src ]; then
  ./djinni/src/run \
    --idl ./core/core.djinni \
    --cpp-out $CORE_CPP_API \
    --cpp-namespace ledger::core::api \
    --cpp-optional-template std::experimental::optional \
    --cpp-optional-header "\"../utils/optional.hpp\"" \
    --jni-include-cpp-prefix "../../api/" \
    --jni-out $CORE_CPP_JNI/jni \
    --node-out $DEST/src \
    --node-type-prefix NJS \
    --node-include-cpp ../include \
    --node-package $PACKAGE_NAME
fi

# copy include files
rm -rf $DEST/include
cp -r $CORE_CPP_API $DEST/include

# copy util files
rm -rf $DEST/utils
mkdir $DEST/utils
cp -r core/src/utils/optional.hpp $DEST/utils

# copy lib files
rm -rf $DEST/lib
mkdir $DEST/lib

# copy dynamic library
cp $CORE_BUILD/core/src/libledger-core.* $DEST/lib

# create tmp folder if needed
if [ ! -e $DEST/js/tmp ]; then
  mkdir $DEST/js/tmp
fi
