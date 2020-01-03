#!/usr/bin/env bash

libcore_path=$1

#Clean secp256k1
echo " >>> Cleaning secp256k1"
rm -rf $libcore_path/core/lib/secp256k1/include $libcore_path/core/lib/secp256k1/src $libcore_path/core/lib/secp256k1/tmp $libcore_path/core/lib/secp256k1/lib

export POLLY_ROOT=$libcore_path/toolchains/polly
export TOOLCHAIN_NAME='linux-gcc-armhf-neon'
export ARCH='armhf'

apt-get -y install g++-arm-linux-gnueabihf qemu-system-arm qemu qemu-user-static
ln -s /usr/arm-linux-gnueabihf/lib/ld-linux-armhf.so.3 /lib/ld-linux-armhf.so.3

cmake -DCMAKE_TOOLCHAIN_FILE=${POLLY_ROOT}/${TOOLCHAIN_NAME}.cmake -DBUILD_TESTS=OFF $libcore_path && make -j4