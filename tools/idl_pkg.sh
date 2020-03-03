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
    CORE_PROJECT=$1 # project library
    CORE_DIR=$CORE_PROJECT
    CORE_IDL_DIR=$CORE_DIR/idl
    CORE_API_DIR=$CORE_DIR/inc/${CORE_PROJECT#ledger-}/api
    CORE_CPP_JNI_DIR=$CORE_DIR/inc/core/jni
    CORE_NODE_DIR=$CORE_DIR/bindings/node
    CORE_NODE_SRC_DIR=$CORE_NODE_DIR/src
    CORE_NODE_PKG_NAME=ledgercore
    CORE_BUILD=ledger-core/build

    echo -e "Generating $CORE_PROJECT JS binding code"

    # recreate node directory
    rm -rf $CORE_NODE_SRC_DIR
    mkdir -p $CORE_NODE_SRC_DIR

    ./djinni/src/run \
        --idl $CORE_IDL_DIR/core.djinni \
        --cpp-out $CORE_API_DIR \
        --cpp-namespace ledger::core::api \
        --cpp-optional-template std::experimental::optional \
        --cpp-optional-header "\"utils/Optional.hpp\"" \
        --node-out $CORE_NODE_SRC_DIR \
        --node-type-prefix NJS \
        --node-include-cpp "../include/core" \
        --node-package $CORE_NODE_PKG_NAME \
        --export-header-name libcore_export \
        --yaml-out $CORE_IDL_DIR \
        --yaml-out-file core.yaml \
        --trace $trace

    # copy include files
    echo "Copying header files…"
    rm -rf $CORE_NODE_DIR/include/core
    mkdir -p $CORE_NODE_DIR/include/core
    cp -r $CORE_API_DIR/* $CORE_NODE_DIR/include/core

    # copy util files
    echo "Copying utils files…"
    rm -rf $CORE_NODE_DIR/include/core/utils
    mkdir -p $CORE_NODE_DIR/include/core/utils
    cp -r ledger-core/inc/core/utils/Optional.hpp $CORE_NODE_DIR/include/core/utils
    cp -r ledger-core/inc/core/LibCoreExport.hpp $CORE_NODE_DIR/include/core

    # copy lib files
    echo "Copying lib files"
    rm -rf $CORE_NODE_DIR/lib
    mkdir -p $CORE_NODE_DIR/lib

    # copy dynamic library
    echo "Copying the dynamic library"
    cp $CORE_BUILD/src/libledger-core.* $CORE_NODE_DIR/lib

    # create tmp folder if needed
    if [ ! -e $CORE_NODE_DIR/js/tmp ]; then
      mkdir -p $CORE_NODE_DIR/js/tmp
    fi
}

case "$2" in
  "npm")
    generate_npm_interface $1
    ;;

  *)
    echo "unknown packaging system"
    exit 1
    ;;
esac

