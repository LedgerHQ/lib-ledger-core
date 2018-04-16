#Check if build direcrtory exists otherwise create it
cd ..
if [ ! -d "lib-ledger-core-build" ]; then
  mkdir lib-ledger-core-build
fi
cd lib-ledger-core-build
#Clean build dir
yes | rm -rf *
#Clean secp256k1 external project
cd  ../lib-ledger-core/core/lib/secp256k1 && rm -rf include src tmp stamp src include lib
#Build library using ios toolchain
cd ../../../../lib-ledger-core-build
../lib-ledger-core/tools/build_ios/ios_build.sh
