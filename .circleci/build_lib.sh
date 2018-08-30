#!/usr/bin/env bash

#export POLLY_ROOT="../lib-ledger-core/toolchains/polly"
export JAVA_HOME="/usr/lib/jvm/java-8-openjdk-i386"
#export ANDROID_NDK_r16b="$HOME/android-ndk-r16b"
export ANDROID_NDK_r16b=/home/circleci/android-ndk-r16b
export POLLY_ROOT="$HOME/lib-ledger-core/toolchains/polly"
echo " >>>>> Check Polly Root "
echo $POLLY_ROOT
ls -la $POLLY_ROOT

cmake \
    -DTARGET_JNI=ON \
    -DCMAKE_BUILD_TYPE:STRING=Release \
    -DBUILD_TESTS=OFF \
    -DCMAKE_TOOLCHAIN_FILE=$POLLY_ROOT/android-ndk-r16b-api-21-x86-clang-libcxx.cmake \
    ../lib-ledger-core

cmake --build . --config Release

#export ANDROID_NDK_r16b="/Users/elkhalilbellakrid/Library/Android/sdk/ndk-bundle"


#echo ">>>> What's inside java home"
##export JAVA_HOME="$(/usr/libexec/java_home -v 1.8)"
#
#export JAVA_HOME="/usr/lib/jvm/java-11-openjdk-amd64/"
#echo "************"
#echo $JAVA_HOME
#echo "************"
##-DCMAKE_CONFIGURATION_TYPES=Release
##-DTARGET_JNI=ON
#cmake -DCMAKE_BUILD_TYPE:STRING=Release -DTARGET_JNI=ON -DCMAKE_TOOLCHAIN_FILE=../lib-ledger-core/toolchains/polly/android-ndk-r16b-api-21-x86-clang-libcxx.cmake ../lib-ledger-core
#cmake --build . --config Release
#
#echo "=====>Create build directory"
#cd .. && mkdir lib-ledger-core-build && cd lib-ledger-core-build
#
#echo "=====>Start build"
#unamestr=`uname`
#BUILD_CONFIG=$1
#echo "======> CMake config for $unamestr in $BUILD_CONFIG mode"
#
#if [ "$BUILD_CONFIG" == "Release" ]; then
#    cmake -DBUILD_TESTS=OFF ../lib-ledger-core
#else
#	if [ "$unamestr" == "Linux" ]; then
#		cmake -DCMAKE_INSTALL_PREFIX=$HOME ../lib-ledger-core
#	elif [ "$unamestr" == "Darwin" ]; then
#		version=`ls /usr/local/Cellar/qt | grep 5.`
#		echo "====> Get qt5 version"
#		echo $version
#		export PATH="/usr/local/Cellar/qt/$version/bin:$PATH"
#		echo $PATH
#	   	cmake -DCMAKE_INSTALL_PREFIX="/usr/local/Cellar/qt/$version" -DCMAKE_PREFIX_PATH="/usr/local/Cellar/qt/$version" ../lib-ledger-core
#	fi
#fi
#
#echo "======> Build for $unamestr in $BUILD_CONFIG mode"
#make -j4

