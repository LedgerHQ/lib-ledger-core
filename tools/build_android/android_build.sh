#!/usr/bin/env bash

arch=$1
libcore_path=$2
android_ndk_16b=$3

if [ "$#" != 3 ]; then
    echo "usage: android_build.sh <arch> <libcore_path> <android_ndk_dir_path>"
    exit 1
fi

#Root to polly toolchains
export POLLY_ROOT=$libcore_path/toolchains/polly
export ANDROID_NDK_r16b=$android_ndk_16b
export ANDROID_NDK_r14=$HOME/Library/Android/sdk/ndk-bundle

export JAVA_HOME="$(/usr/libexec/java_home -v 1.8)"

export ARCH=$arch
if [ "$ARCH" == "armeabi-v7a" ]; then
    export TOOLCHAIN_NAME='android-ndk-r16b-api-21-armeabi-v7a-clang-libcxx14'
  elif [ "$ARCH" == "arm64-v8a" ]; then
    export TOOLCHAIN_NAME='android-ndk-r16b-api-21-arm64-v8a-neon-clang-libcxx14'
  else
    export TOOLCHAIN_NAME='android-ndk-r16b-api-21-x86-clang-libcxx'
fi

cmake . -DCMAKE_BUILD_TYPE:STRING=Release -DTARGET_JNI=ON -DCMAKE_TOOLCHAIN_FILE=${POLLY_ROOT}/${TOOLCHAIN_NAME}.cmake
cmake --build . --config Release -- -j8

