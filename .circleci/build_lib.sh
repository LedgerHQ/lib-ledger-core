#!/usr/bin/env bash

###
# Global variables (may be used everywhere in this script)
##

BUILD_CONFIG="Debug"
cmake_params="" # All params passed to cmake to generate build files
export ARCH=$2

###
# Commands of the script. Put them in command line parameters to trigger
###

function command_target_jni {
  add_to_cmake_params -DTARGET_JNI=ON
}

function command_Release {
  BUILD_CONFIG="Release"
  add_to_cmake_params -DBUILD_TESTS=OFF
}

function command_Debug {
  BUILD_CONFIG="Debug"
  add_to_cmake_params -DBUILD_TESTS=ON
}

function command_ios {
  export POLLY_IOS_BUNDLE_IDENTIFIER='com.ledger.core'
  #Needed for nocodesign toolchains
  export XCODE_XCCONFIG_FILE=$POLLY_ROOT/scripts/NoCodeSign.xcconfig
  echo "command_ios with architecture : $ARCH"
  if [ "$ARCH" == "armv7" ]; then
    export TOOLCHAIN_NAME='ios-nocodesign-11-2-dep-9-3-armv7'
    export OSX_SYSROOT=iphoneos
  elif [ "$ARCH" == "arm64" ]; then
    export TOOLCHAIN_NAME='ios-nocodesign-11-2-dep-9-3-arm64'
    export OSX_SYSROOT=iphoneos
  else
    export TOOLCHAIN_NAME='ios-nocodesign-11-2-dep-9-3'
    export OSX_SYSROOT=iphonesimulator
    export ARCH=x86_64
    #Copy iphone.cmake which is not forcing CMAKE_OSX_SYSROOT to iphoneos in cache
    cp `pwd`/../lib-ledger-core/tools/build_ios/iphone.cmake `pwd`/../lib-ledger-core/toolchains/polly/os/
  fi

  cp `pwd`/../lib-ledger-core/tools/build_ios/framework.plist.in `pwd`
  cp `pwd`/../lib-ledger-core/tools/build_ios/install_name.sh `pwd`

  echo 'Fixing NoCodeSign (see https://github.com/ruslo/polly/issues/302 for further details)'
  sed -i '' '/CODE_SIGN_ENTITLEMENTS/d' $(pwd)/../lib-ledger-core/toolchains/polly/scripts/NoCodeSign.xcconfig

  BUILD_CONFIG="Release"
  add_to_cmake_params -G "Xcode" -DCMAKE_BUILD_TYPE:STRING=Release -DBUILD_TESTS=OFF -DCMAKE_OSX_ARCHITECTURES:STRING=${ARCH} -DCMAKE_MACOSX_BUNDLE:BOOL=ON -DCMAKE_OSX_SYSROOT:STRING=${OSX_SYSROOT} -DCMAKE_TOOLCHAIN_FILE=${POLLY_ROOT}/${TOOLCHAIN_NAME}.cmake
}

function command_android {
  echo "Set Android NDK variable"
  export ANDROID_NDK_r16b=/home/circleci/android-ndk-r16b
  export JAVA_HOME="$(/usr/libexec/java_home -v 1.8)" || export JAVA_HOME="/usr/lib/jvm/java-11-openjdk-amd64/"
  #Needed for nocodesign toolchains
  echo "command_android with architecture : $ARCH"
  if [ "$ARCH" == "armeabi-v7a" ]; then
    export TOOLCHAIN_NAME='android-ndk-r16b-api-21-armeabi-v7a-clang-libcxx14'
  elif [ "$ARCH" == "arm64-v8a" ]; then
    export TOOLCHAIN_NAME='android-ndk-r16b-api-21-arm64-v8a-neon-clang-libcxx14'
  else
    export TOOLCHAIN_NAME='android-ndk-r16b-api-21-x86-clang-libcxx'
  fi
  BUILD_CONFIG="Release"
  add_to_cmake_params -DCMAKE_BUILD_TYPE:STRING=Release -DBUILD_TESTS=OFF -DTARGET_JNI=ON -DCMAKE_TOOLCHAIN_FILE=${POLLY_ROOT}/${TOOLCHAIN_NAME}.cmake
}

###
# Utilities functions
###

function add_to_cmake_params {
  local param
  for param; do
    cmake_params="$cmake_params $param"
  done
}

function execute_commands {
if [ "$1" == "ios" -o "$1" == "android" ]; then
    command_$1
else
  local cmd
  for cmd;do
    echo $cmd
    command_$cmd
  done
fi

}

export JAVA_HOME="/usr/lib/jvm/java-8-openjdk"
export POLLY_ROOT=`pwd`/toolchains/polly
###
# Clean
###
echo "=====>Cleaning SQLCipher"
rm -rf core/lib/sqlcipher || echo "Failed to clean sqlcipher"
if [ "$1" == "ios" -o "$1" == "android" ]; then
    echo "=====>Cleaning to prepare clean build"
    echo "=====>Cleaning secp256k1"
    rm -rf core/lib/secp256k1/include core/lib/secp256k1/src core/lib/secp256k1/tmp core/lib/secp256k1/lib || echo "Failed to clean secp256k1"
    echo "=====>Remove build directory"
    rm -rf ../lib-ledger-core-build
fi

echo "=====>Create which will contain artifacts"
cd .. && (mkdir lib-ledger-core-artifacts || echo "lib-ledger-core-artifacts directory already exists")

echo "=====>Create build directory"
(mkdir lib-ledger-core-build || echo "lib-ledger-core-build directory already exists") && cd lib-ledger-core-build

echo "=====>Start build"
unamestr=`uname`
execute_commands $*

echo "======> CMake config for $unamestr in $BUILD_CONFIG mode"

if [ "$BUILD_CONFIG" == "Debug" ]; then
    if [ "$unamestr" == "Linux" ]; then
    add_to_cmake_params "-DCMAKE_PREFIX=$HOME"
    elif [ "$unamestr" == "Darwin" ]; then
        version=`ls /usr/local/Cellar/qt | grep 5.`
        echo "====> Get qt5 version"
        echo $version
        export PATH="/usr/local/Cellar/qt/$version/bin:$PATH"
        echo $PATH
    add_to_cmake_params -DCMAKE_INSTALL_PREFIX="/usr/local/Cellar/qt/$version" -DCMAKE_PREFIX_PATH="/usr/local/Cellar/qt/$version"
    fi
fi

if [ "$1" == "ios" ]; then
    echo "Resetting xcode tools..."

    # The reset is necessary for obscure reasons not to sign on iOS (maybe the CI breaks the PATH or
    # something). See the fix below that removes signatures.
    sudo xcode-select --reset
fi

echo $cmake_params
cmake $cmake_params ../lib-ledger-core
echo "======> Build for $unamestr in $BUILD_CONFIG mode"
if [ "$1" == "ios" ]; then
    echo " >>> Starting iOS build for architecture ${ARCH} with toolchain ${TOOLCHAIN_NAME} for ${OSX_SYSROOT}"
    xcodebuild -project ledger-core.xcodeproj -configuration Release -jobs 4
else
    make -j4
fi
