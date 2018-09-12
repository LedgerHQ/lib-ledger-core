#!/usr/bin/env bash

#Clean secp256k1
echo " >>> Cleaning secp256k1"
rm -rf ../lib-ledger-core/core/lib/secp256k1/include ../lib-ledger-core/core/lib/secp256k1/src ../lib-ledger-core/core/lib/secp256k1/tmp ../lib-ledger-core/core/lib/secp256k1/lib

#Root to polly toolchains
export POLLY_ROOT=`pwd`/../lib-ledger-core/toolchains/polly
export ANDROID_NDK_r16b=${HOME}/Library/Android/sdk/ndk-bundle
export ANDROID_NDK_r14=${HOME}/Library/Android/sdk/ndk-bundle

export JAVA_HOME="$(/usr/libexec/java_home -v 1.8)" || export JAVA_HOME="/usr/lib/jvm/java-11-openjdk-amd64/"
export JAVA_HOME="$(/usr/libexec/java_home -v 1.8)"

export ARCH=$1
if [ "${ARCH}" == 'armeabi-v7a' ]; then
	export TOOLCHAIN_NAME='android-ndk-r16b-api-21-armeabi-v7a-clang-libcxx14'
elif [ "${ARCH}" == 'arm64-v8a' ]; then
	export TOOLCHAIN_NAME='android-ndk-r16b-api-24-arm64-v8a-clang-libcxx14'
else
	export TOOLCHAIN_NAME='android-ndk-r16b-api-16-x86-clang-libcxx14'
fi

cmake -DCMAKE_BUILD_TYPE:STRING=Release -DTARGET_JNI=ON -DCMAKE_TOOLCHAIN_FILE=${POLLY_ROOT}/${TOOLCHAIN_NAME}.cmake ../lib-ledger-core
cmake --build . --config Release

