#!/usr/bin/env bash
#There is an issue when building with arm64 and armv7
#During the cmake build this script fails,
#but works when executed from terminal,
#only possible reason is that the environment set by cmake
#is 'polluting' somehow the env and makes it fail
#Workaround: use prebuilt sqlcipher library for iOS (armv7 and arm64)
ARCH=$1
DIST_DIR=build_ios_$ARCH
SQLCIPHER_SRC_DIR=$2
#LOCAL_PATH=$PATH
#export PATH=$3
IOS_MIN_SDK_VERSION="10.0"

SQLCIPHER_IOS=$SQLCIPHER_SRC_DIR
mkdir -p "$SQLCIPHER_IOS"
pushd "$SQLCIPHER_IOS"

if [[ "${ARCH}" == "i386" || "${ARCH}" == "x86_64" ]]; then
OS_COMPILER="iPhoneSimulator"
if [[ "${ARCH}" == "x86_64" ]]; then
HOST="x86_64-apple-darwin"
else
HOST="x86-apple-darwin"
fi
elif [[ "${ARCH}" == "armv7" || "${ARCH}" == "arm64" ]]; then
OS_COMPILER="iPhoneOS"
HOST="arm-apple-darwin"
else
echo "Unsupported architecture"
exit 1
fi

DEVELOPER=$(xcode-select -print-path)
export CROSS_TOP="${DEVELOPER}/Platforms/${OS_COMPILER}.platform/Developer"
export CROSS_SDK="${OS_COMPILER}.sdk"
TOOLCHAIN_BIN="${DEVELOPER}/Toolchains/XcodeDefault.xctoolchain/usr/bin"
export CC="${TOOLCHAIN_BIN}/clang"
export AR="${TOOLCHAIN_BIN}/ar"
export RANLIB="${TOOLCHAIN_BIN}/ranlib"
export STRIP="${TOOLCHAIN_BIN}/strip"
export LIBTOOL="${TOOLCHAIN_BIN}/libtool"
export NM="${TOOLCHAIN_BIN}/nm"
export LD="${TOOLCHAIN_BIN}/ld"

CFLAGS="\
-fembed-bitcode \
-arch ${ARCH} \
-isysroot ${CROSS_TOP}/SDKs/${CROSS_SDK} \
-mios-version-min=${IOS_MIN_SDK_VERSION} \
"

SQLCIPHER_CFLAGS=" \
-DSQLITE_HAS_CODEC \
-DSQLITE_SOUNDEX \
-DSQLITE_THREADSAFE=1 \
-DSQLITE_ENABLE_MEMORY_MANAGEMENT=1 \
-DSQLITE_ENABLE_DBSTAT_VTAB \
-DSQLITE_SECURE_DELETE \
"

#printenv

./configure \
--with-pic \
--disable-tcl \
--disable-shared \
--host="$HOST" \
--verbose \
--with-crypto-lib=commoncrypto \
--enable-tempstore=yes \
--enable-threadsafe=yes \
--disable-editline \
CFLAGS="${CFLAGS} ${SQLCIPHER_CFLAGS}" \
LDFLAGS="-framework Security -framework Foundation"

#export PATH=$LOCAL_PATH
popd