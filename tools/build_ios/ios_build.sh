#!/usr/bin/env bash

#Clean secp256k1
echo " >>> Cleaning secp256k1"
rm -rf ../lib-ledger-core/core/lib/secp256k1/include ../lib-ledger-core/core/lib/secp256k1/src ../lib-ledger-core/core/lib/secp256k1/tmp ../lib-ledger-core/core/lib/secp256k1/lib

#WARNING: for iphonesimulator build don't forget to remove FORCE in iphone.cmake (polly toolchain)
#ID used for bundle
export POLLY_IOS_BUNDLE_IDENTIFIER='com.ledger.core'
#Root to polly toolchains
export POLLY_ROOT=`pwd`/../lib-ledger-core/toolchains/polly
#Needed for nocodesign toolchains
export XCODE_XCCONFIG_FILE=$POLLY_ROOT/scripts/NoCodeSign.xcconfig


export ARCH=$1
if [ "${ARCH}" == 'armv7' ]; then
	export TOOLCHAIN_NAME='ios-nocodesign-11-2-dep-9-3-armv7'
	export OSX_SYSROOT=iphoneos
elif [ "${ARCH}" == 'arm64' ]; then
	export TOOLCHAIN_NAME='ios-nocodesign-11-2-dep-9-3-arm64'
	export OSX_SYSROOT=iphoneos
elif [ "${ARCH}" == 'bitcode' ]; then
	#ios-nocodesign-11-2-dep-9-0-bitcode-cxx11
	export TOOLCHAIN_NAME='ios-nocodesign-11-2-dep-9-0-bitcode-cxx11'
	#export TOOLCHAIN_NAME='ios-nocodesign-11-2-dep-9-3-arm64-armv7'
else
	export TOOLCHAIN_NAME='ios-nocodesign-11-2-dep-9-3'
	export OSX_SYSROOT=iphonesimulator
	export ARCH=x86_64
	cp `pwd`/../lib-ledger-core/tools/build_ios/iphone.cmake $POLLY_ROOT/os/
fi

cp `pwd`/../lib-ledger-core/tools/build_ios/framework.plist.in `pwd`

cp `pwd`/../lib-ledger-core/tools/build_ios/install_name.sh `pwd`



echo " >>> Starting iOS build for architecture ${ARCH} with toolchain ${TOOLCHAIN_NAME} for ${OSX_SYSROOT}"
if [ "${ARCH}" == 'bitcode' ]; then
	cmake -G "Xcode" -DCMAKE_BUILD_TYPE=Release -DCMAKE_MACOSX_BUNDLE:BOOL=ON -DCMAKE_TOOLCHAIN_FILE=${POLLY_ROOT}/${TOOLCHAIN_NAME}.cmake -DBUILD_TESTS=OFF ../lib-ledger-core
else
	cmake -G "Xcode" -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES:STRING=${ARCH} -DCMAKE_MACOSX_BUNDLE:BOOL=ON -DCMAKE_OSX_SYSROOT:STRING=${OSX_SYSROOT} -DCMAKE_TOOLCHAIN_FILE=${POLLY_ROOT}/${TOOLCHAIN_NAME}.cmake -DBUILD_TESTS=OFF ../lib-ledger-core
fi
xcodebuild -project ledger-core.xcodeproj -configuration Release -jobs 4
