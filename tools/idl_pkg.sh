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

  for coin in $*; do
    if [ "$coin" == "core" ]; then
      PROJECT_NAME=ledger-core
      NODE_PKG_NAME=ledgercore
    else
      PROJECT_NAME=ledger-core-$coin
      NODE_PKG_NAME=ledgercore-$coin
    fi

    CURRENCY_NAME=$coin
    API_DIR=$PROJECT_NAME/inc/$CURRENCY_NAME/api
    CPP_JNI_DIR=$PROJECT_NAME/inc/$CURRENCY_NAME/jni
    BUILD=$PROJECT_NAME/build

    echo -e "Generating $PROJECT_NAME ($CURRENCY_NAME) JS binding code"

    ./djinni/src/run \
      --idl $PROJECT_NAME/idl/idl.djinni \
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
    cp $BUILD/src/lib$PROJECT_NAME.* $NODE_DIR/lib
  done

  # Gather all #include
  echo "Generating the final output C++ one-source concatenated interface"
  OUTPUT_CPP=$NODE_SRC_DIR/final-output.cpp

  (find $NODE_SRC_DIR -type f -name "ledgercore*cpp" -exec grep "\#include" '{}' \;) > $OUTPUT_CPP
  echo "using namespace v8;" >> $OUTPUT_CPP
  echo "using namespace node;" >> $OUTPUT_CPP
  echo "static void initAll(Local<Object> target) {" >> $OUTPUT_CPP
  echo -e "    Nan::HandleScope scope;" >> $OUTPUT_CPP
  (find $NODE_SRC_DIR -type f -name "ledgercore*cpp" -exec grep "Initialize(target)" '{}' \; | awk '!x[$0]++') >> $OUTPUT_CPP
  echo "}" >> $OUTPUT_CPP
  echo "NODE_MODULE(ledgercore,initAll);" >> $OUTPUT_CPP

  echo "Removing temporary C++ source interface"
  rm $NODE_SRC_DIR/ledgercore*cpp

  echo "Generating the binding.gyp file…"
  ./tools/gyp_generator.py $*
}

case "$1" in
  "npm")
    shift
    generate_npm_interface $*
    ;;

  *)
    echo "unknown packaging system"
    exit 1
    ;;
esac
