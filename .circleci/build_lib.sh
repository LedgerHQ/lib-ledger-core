#!/usr/bin/env bash

###
# Global variables (may be used everywhere in this script)
##

BUILD_CONFIG="Debug"
cmake_params="" # All params passed to cmake to generate build files

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
  local cmd
  for cmd;do
    echo $cmd
    command_$cmd
  done
}

export JAVA_HOME="/usr/lib/jvm/java-8-openjdk"

echo "=====>Create build directory"
cd .. && (mkdir lib-ledger-core-build || echo "lib-ledger-core-build directory already exists") && cd lib-ledger-core-build

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

echo $cmake_params
cmake $cmake_params ../lib-ledger-core
echo "======> Build for $unamestr in $BUILD_CONFIG mode"
make -j4
