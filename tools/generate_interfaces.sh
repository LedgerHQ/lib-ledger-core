#!/usr/bin/env bash
# Generate library API with djinni

echo "Generating core API"

CORE_CPP_API_DIRECTORY=core/src/api
CORE_CPP_JNI_DIRECTORY=core/src/jni
rm -rf $CORE_CPP_API_DIRECTORY $CORE_CPP_JNI_DIRECTORY
./djinni/src/run    --idl ./core/core.idl \
                    --cpp-out $CORE_CPP_API_DIRECTORY \
                    --jni-include-cpp-prefix "../../api/" \
                    --jni-out $CORE_CPP_JNI_DIRECTORY/jni \
                    --java-out api/core/java

cp ./djinni/support-lib/jni/* $CORE_CPP_JNI_DIRECTORY/jni
cp ./djinni/support-lib/*.hpp $CORE_CPP_JNI_DIRECTORY


echo
echo "Generating device API"
rm -rf device/src/api
./djinni/src/run --idl ./device/device.idl --cpp-out device/src/api

echo "Generating wallet API"