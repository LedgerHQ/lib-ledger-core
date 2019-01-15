#!/usr/bin/env bash

PACKAGE_NAME=ledgercore
DEST=$1
CORE_BUILD=$2

CORE_CPP_API=core/src/api
CORE_CPP_JNI=core/src/jni

if [[ "$*" == "--help" || "$#" != "2" ]];
then
  echo "usage: ./generateBindings.sh <dest> <core_build>"
  echo -e "\tdest:       The node bindings target destination. This is"
  echo -e "\t            typically the cloned repository that contains"
  echo -e "\t            the node bindings for lib-core."
  echo -e "\tcore_build: The path to your lib-core build directory."
  exit 0
fi

./djinni/src/run \
  --idl ./core/core.djinni \
  --cpp-out $DEST/include \
  --cpp-namespace ledger::core::api \
  --cpp-optional-template std::experimental::optional \
  --cpp-optional-header "\"../utils/optional.hpp\"" \
  --jni-include-cpp-prefix "../../api/" \
  --jni-out $CORE_CPP_JNI/jni \
  --node-out $DEST/src \
  --node-type-prefix NJS \
  --node-include-cpp ../include \
  --node-package $PACKAGE_NAME

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
