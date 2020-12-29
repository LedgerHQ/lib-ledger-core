#!/usr/bin/env bash

###
# Global variables (may be used everywhere in this script)
##

BUILD_CONFIG="Debug"
cmake_params="" # All params passed to cmake to generate build files
unamestr=`uname`
export ARCH=$2
export CMAKE=cmake
export PATH=$PATH:~/cmake_folder/bin
export PG_INCLUDE_DIR=`[[ "$unamestr" = "Darwin" ]] && echo -n "/usr/local/opt/postgresql/include" || echo -n "/usr/include/postgresql"`

echo "Use $PG_INCLUDE_DIR for PGSQL"

###
# Commands of the script. Put them in command line parameters to trigger
###

function command_target_jni {
  if [[ "$unamestr" = "Darwin" ]]; then
    export JAVA_HOME="$($(dirname $(readlink $(which javac)))/java_home)"
    add_to_cmake_params -DOPENSSL_ROOT_DIR="/usr/local/opt/openssl"
  else
    export JAVA_HOME="$(dirname $(dirname $(readlink -f $(which javac))))"
    add_to_cmake_params -DSYS_OPENSSL=ON
  fi
  add_to_cmake_params -DTARGET_JNI=ON  -DSSL_SUPPORT=ON -DPG_SUPPORT=ON -DPostgreSQL_INCLUDE_DIR="$PG_INCLUDE_DIR" #ACTIVATIN SSL ONLY WHEN USING PG FOR WD
}

function command_Release {
  BUILD_CONFIG="Release"
  add_to_cmake_params -DBUILD_TESTS=OFF
}

function command_Debug {
  BUILD_CONFIG="Debug"
  add_to_cmake_params -DBUILD_TESTS=ON -DPG_SUPPORT=ON -DPostgreSQL_INCLUDE_DIR=/usr/include/postgresql -DSYS_OPENSSL=ON -DOPENSSL_USE_STATIC_LIBS=TRUE 
}

function command_arch_ssl_1_1 {
  add_to_cmake_params "-DCMAKE_PREFIX=$HOME" "-DCMAKE_BUILD_TYPE=Release" "-DSYS_OPENSSL=ON" "-DOPENSSL_USE_STATIC_LIBS=TRUE"
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
  export ANDROID_NDK_r18b=/home/circleci/android-ndk-r18b
  export JAVA_HOME="$(/usr/libexec/java_home -v 1.8)" || export JAVA_HOME="/usr/lib/jvm/java-11-openjdk-amd64/"
  #Needed for nocodesign toolchains
  echo "command_android with architecture : $ARCH"
  if [ "$ARCH" == "armeabi-v7a" ]; then
    export TOOLCHAIN_NAME='android-ndk-r18b-api-21-armeabi-v7a-clang-libcxx'
  elif [ "$ARCH" == "arm64-v8a" ]; then
    export TOOLCHAIN_NAME='android-ndk-r18b-api-21-arm64-v8a-clang-libcxx'
  elif [ "$ARCH" == "x86_64" ]; then
    export TOOLCHAIN_NAME='android-ndk-r18b-api-21-x86-64-clang-libcxx'
  else
      export TOOLCHAIN_NAME='android-ndk-r18b-api-21-x86-clang-libcxx'
  fi
  #This is useful for SQLCipher/config.guess
  export LIBC=gnu
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

export JAVA_HOME="/usr/lib/jvm/java-11-openjdk-amd64/"
export POLLY_ROOT=`pwd`/toolchains/polly
###
# Clean
###
if [ "$1" == "ios" -o "$1" == "android" ]; then
    echo "=====>Cleaning to prepare clean build"
    echo "=====>Remove build directory"
    rm -rf ../lib-ledger-core-build
fi

echo "=====>Create which will contain artifacts"
cd .. && (mkdir lib-ledger-core-artifacts || echo "lib-ledger-core-artifacts directory already exists")

echo "=====>Create build directory"
(mkdir lib-ledger-core-build || echo "lib-ledger-core-build directory already exists") && cd lib-ledger-core-build

echo "=====>Start build"
execute_commands $*

echo "======> CMake config for $unamestr in $BUILD_CONFIG mode"

if [ "$BUILD_CONFIG" == "Debug" ]; then
    if [ "$unamestr" == "Linux" ]; then
        add_to_cmake_params "-DCMAKE_PREFIX=$HOME" "-DCMAKE_BUILD_TYPE=Debug" 
    elif [ "$unamestr" == "Darwin" ]; then
        add_to_cmake_params -DPostgreSQL_INCLUDE_DIR="$PG_INCLUDE_DIR" 
        add_to_cmake_params "-DOPENSSL_ROOT_DIR=/usr/local/opt/openssl/"
    fi
fi

if [ "$1" == "ios" ]; then
    echo "Resetting xcode tools..."

    # The reset is necessary for obscure reasons not to sign on iOS (maybe the CI breaks the PATH or
    # something). See the fix below that removes signatures.
    sudo xcode-select --reset
fi

if [ "$BUILD_CONFIG" == "Debug" ]; then
	echo "======> Create PostgreSQL with name test_db, host localhost and port 5432"
	export POSTGRES_USER=postgres
	export POSTGRES_DB=test_db
	if [ "$unamestr" == "Linux" ]; then
		# This modification is done to avoid authentication
		echo "======> Modify PostgreSQL configuration file ..."
		sed 's/md5/trust/g' /etc/postgresql/11/main/pg_hba.conf > pg_hba.conf.mod
		mv pg_hba.conf.mod /etc/postgresql/11/main/pg_hba.conf
		sed 's/peer/trust/g' /etc/postgresql/11/main/pg_hba.conf > pg_hba.conf.mod
		mv pg_hba.conf.mod /etc/postgresql/11/main/pg_hba.conf
		echo "======> Create database ..."
		service postgresql start
		createuser root -s -U postgres
		psql -c "create database test_db" -U postgres -h localhost -p 5432
	elif [ "$unamestr" == "Darwin" ]; then
		mkdir test_db
		initdb -D test_db
		pg_ctl start -D test_db -l db_log -o "-i -h localhost -p 5432"
		createdb -h localhost -p 5432 test_db
	fi
fi

echo $cmake_params
cmake $cmake_params ../lib-ledger-core
echo "======> Build for $unamestr in $BUILD_CONFIG mode"
if [ "$1" == "ios" ]; then
    echo " >>> Starting iOS build for architecture ${ARCH} with toolchain ${TOOLCHAIN_NAME} for ${OSX_SYSROOT}"
    xcodebuild -project ledger-core.xcodeproj -configuration Release -jobs 4
else
    make -j16 -l2
fi
