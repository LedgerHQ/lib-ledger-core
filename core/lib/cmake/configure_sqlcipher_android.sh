#!/usr/bin/env bash
set -euvx

if [ "$#" -ne 5 ]
then
    echo "Usage:"
    echo "./configure_sqlcipher_android.sh <ABSOLUTE_SRC_DIR> <TOOLCHAIN> <ANDROID_NDK_PATH> <OPENSSL_SRC_DIR> <OPENSSL_LIB>"
    exit 1
fi

SQLCIPHER_DIR=$1/sqlcipher/src/SQLCipher
DIST_DIR=$SQLCIPHER_DIR
TOOLCHAIN=$2
ANDROID_NDK_PATH=$3
TOOLCHAIN_PATH=$ANDROID_NDK_PATH/toolchains/$TOOLCHAIN-4.9
if [ "$TOOLCHAIN" == "i686-linux-android" ]; then
	TOOLCHAIN_PATH=$ANDROID_NDK_PATH/toolchains/x86-4.9
fi
unamestr=`uname`
if [ "$unamestr" == "Darwin" ]; then
	TOOLCHAIN_PATH=$TOOLCHAIN_PATH/prebuilt/darwin-x86_64
elif [ "$unamestr" == "Linux" ]; then
	TOOLCHAIN_PATH=$TOOLCHAIN_PATH/prebuilt/linux-x86_64
fi

ANDROID_NDK_API_VERSION="21"
OPENSSL_SRC_DIR=$4
OPENSSL_LIB=$5

cd "${SQLCIPHER_DIR}"

export TOOLCHAIN_BIN="$TOOLCHAIN_PATH""/bin/"
export CC="$TOOLCHAIN_BIN""$TOOLCHAIN""-gcc"
export CXX="$TOOLCHAIN_BIN""$TOOLCHAIN""-g++"
export RANLIB="$TOOLCHAIN_BIN""$TOOLCHAIN""-ranlib"
export LD="$TOOLCHAIN_BIN""$TOOLCHAIN""-ld"
export AR="$TOOLCHAIN_BIN""$TOOLCHAIN""-ar"

if [ "$TOOLCHAIN" == "i686-linux-android" ]
then
  ARCH="x86"
  HOST="i686-linux"
elif [ "$TOOLCHAIN" == "aarch64-linux-android" ]
then
  ARCH="arm64"
  HOST="arm-linux"
elif [ "$TOOLCHAIN" == "arm-linux-androideabi" ]
then
  ARCH="arm"
  HOST="arm-linux"
else
  echo "Unknown toolchain"
  exit 1
fi

SYS_ROOT=$ANDROID_NDK_PATH/platforms/android-$ANDROID_NDK_API_VERSION/arch-$ARCH

export CFLAGS="-D__ANDROID_API__=$ANDROID_NDK_API_VERSION --sysroot=$SYS_ROOT -I$ANDROID_NDK_PATH/sysroot/usr/include/$TOOLCHAIN -I$ANDROID_NDK_PATH/sysroot/usr/include"
export CPPFLAGS="-I$ANDROID_NDK_PATH/sysroot/usr/include/$TOOLCHAIN -I$ANDROID_NDK_PATH/sysroot/usr/include"
SQLCIPHER_CFLAGS=" \
  -DSQLITE_HAS_CODEC \
  -DSQLITE_THREADSAFE=1 \
  -DSQLITE_ENABLE_MEMORY_MANAGEMENT=1 \
"

./configure \
  --host="${HOST}" \
  --with-pic \
  --disable-tcl \
  --disable-shared \
  --enable-tempstore=yes \
  CFLAGS="${CFLAGS} ${SQLCIPHER_CFLAGS} -I${OPENSSL_SRC_DIR}/include -L${OPENSSL_LIB}" \
  CPPFLAGS="${CPPFLAGS}" \
  LIBS="-lcrypto" \
  LDFLAGS="-L${OPENSSL_LIB}"