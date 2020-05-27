#!/usr/bin/env bash
#
# Generate library API with djinni.
# Invocate with TRACE=1 as environment variable to set debug compilation.

set -e

if [[ $TRACE ]] ; then
  echo "Debug compilation enabled"
  trace="true";
else
  trace="false";
fi

# Generate the coins.djinni file.
BUNDLE_IDL_FILE=bundle/bundle.djinni
COINS_IDL_FILE=bundle/coins.djinni
rm -f $COINS_IDL_FILE
echo $*
for coin in $*; do
  echo "@import \"../ledger-core-$coin/idl/idl.djinni\"" >> $COINS_IDL_FILE
done

OUTPUT_API_DIR=bundle/api
OUTPUT_JNI_DIR=bundle/jni
OUTPUT_JAVA_DIR=bundle/java
OUTPUT_SCALA_DIR=bundle/scala

# recreate API directory
rm -rf OUTPUT_API_DIR
rm -rf OUTPUT_JNI_DIR
rm -rf OUTPUT_JAVA_DIR
rm -rf OUTPUT_SCALA_DIR
mkdir OUTPUT_API_DIR
mkdir OUTPUT_JNI_DIR
mkdir OUTPUT_JAVA_DIR
mkdir OUTPUT_SCALA_DIR

./djinni/src/run \
  --idl $BUNDLE_IDL_FILE \
  --cpp-out $OUTPUT_API_DIR \
  --cpp-namespace ledger::core::api \
  --cpp-optional-template std::experimental::optional \
  --cpp-optional-header "<core/utils/Optional.hpp>" \
  --export-header-name libcore_export \
  --jni-include-cpp-prefix "../../api/" \
  --jni-out $OUTPUT_JNI_DIR \
  --java-out $OUTPUT_JAVA_DIR \
  --java-package co.ledger.core \
  --trace $trace
