#!/usr/bin/env bash
set -e

BUILD_TYPE=Release
if [ ! -z "$1" ];
then
  BUILD_TYPE=$1
fi

cd ~
git clone --recursive https://github.com/LedgerHQ/lib-ledger-core.git
export CORE_SSL_1_1=arch_ssl_1_1
mkdir -p ~/lib-ledger-core-artifacts
cd ~/lib-ledger-core
.circleci/build_lib.sh target_jni $BUILD_TYPE
export LIB_VERSION=`.circleci/export_lib_version.sh`
. .circleci/copy_artifacts.sh target_jni

. tools/generate_interfaces.sh

mkdir -p ~/lib-ledger-core-artifacts/macos/jni
touch ~/lib-ledger-core-artifacts/macos/jni/libledger-core_jni.dylib

CIRCLE_TAG=mytag .circleci/publish_jar.sh

cp /root/lib-ledger-core-artifacts/ledger-lib-core.jar /hostdir/

