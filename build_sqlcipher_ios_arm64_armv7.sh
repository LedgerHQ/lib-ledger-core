#!/usr/bin/env bash
if [ "$#" -ne 1 ]
then
echo "Usage:"
echo "./build_sqlcipher_ios_armv64_amv7.sh <ARCH>"
exit 1
else
echo "Building sqlcipher for $1 in core/lib/sqlcipher/src/SQLCipher directory"
fi

rm -rf core/lib/sqlcipher
mkdir -p core/lib/sqlcipher/src/SQLCipher
git clone https://github.com/SQLCipher/SQLCipher.git core/lib/sqlcipher/src/SQLCipher

LIBS_DIR=core/lib
SQLCIPHER_SRC_DIR=$LIBS_DIR/sqlcipher/src/SQLCipher

pushd $SQLCIPHER_SRC_DIR
./../../../cmake/configure_sqlcipher_ios.sh $1 .
./../../../cmake/build_sqlcipher_ios.sh
popd

mkdir -p sqlcipher-$1-tmp/include/sqlcipher
mkdir sqlcipher-$1-tmp/lib

cp $SQLCIPHER_SRC_DIR/sqlite3*.h sqlcipher-$1-tmp/include/sqlcipher
cp $SQLCIPHER_SRC_DIR/.libs/libsqlcipher.a sqlcipher-$1-tmp/lib
