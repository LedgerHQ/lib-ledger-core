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
}

case "$1" in
  "npm")
    generate_npm_interface
    ;;

  *)
    echo "unknown packaging system"
    exit 1
    ;;
esac
