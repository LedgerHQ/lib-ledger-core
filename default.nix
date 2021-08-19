{ release ? true, jni ? true, pushS3 ? false, runTests ? false }:

let
  pkgs = import ./nix/pkgs.nix { jdk = "jdk8"; };
  pinned = import ./nix/pinned.nix;
  inherit (import (pinned.gitignoreSrc { inherit pkgs; }) { inherit (pkgs) lib; }) gitignoreSource;
  secp256k1-chfast = import ./nix/secp256k1.nix { inherit pkgs; };
  buildTypeFlag = if release
  then [
    "-DCMAKE_BUILD_TYPE=RELEASE"
  ]
  else [
    "-DCMAKE_BUILD_TYPE=DEBUG"
  ];
  testFlag = if jni && runTests
  then abort "Cannot build both for JNI and run the test suite. Choose only one."
  else if runTests
  then [ "-DBUILD_TESTS=ON" ]
  else [ "-DBUILD_TESTS=OFF" ];
  jniFlag = [ (pkgs.lib.optionalString jni "-DTARGET_JNI=ON") ];
in

pkgs.stdenv.mkDerivation {
  name = "libledger-core";
  version = "4.1.1";
  src = gitignoreSource ./.;

  nativeBuildInputs = (pkgs.lib.attrVals [
    "pkg-config"
    "cmake"
    "aws"
  ] pkgs);

  buildInputs = (pkgs.lib.attrVals [
    # Common build deps
    "postgresql_12"
    "openssl_1_1"
    "gcc10"
    "sqlite"
    "cmake"
    "libkrb5"
    "cryptopp"

    # JNI bindings deps
    "jdk8"
  ] pkgs)
  ++ [ secp256k1-chfast ]
  ++ pkgs.lib.optionals pkgs.stdenv.isLinux [ pkgs.libselinux ] ;

  cmakeFlags = buildTypeFlag
  ++ testFlag
  ++ jniFlag
  ++ [
    "-DPG_SUPPORT=ON"
    "-DSYS_OPENSSL=ON"
    "-DSYS_SECP256K1=ON"
  ];

  doInstallCheck = runTests;
  installCheckPhase = ''
    echo "========= Provisioning Database"
    initdb -D .tmp/mydb
    pg_ctl -D .tmp/mydb -l logfile -o "--unix_socket_directories='$PWD'" start
    createdb -p 5432 -h localhost test_db
    echo "========= Running tests"
    ctest --output-on-failure
  '';

  separateDebugInfo = true;

  doDist = pushS3;
  tarballs = [
    "*.so"
    "*.dylib"
  ];
  distPhase = ''
    function match {
      if [[ "$OSTYPE" == "darwin"* ]]; then
        sed -nE $*
      else
        sed -nr $*
      fi
    }

    LIB_VERSION_MAJOR=`cat CMakeLists.txt | match 's/set\(VERSION_MAJOR.*([0-9]+).*\)/\1/p'`
    LIB_VERSION_MINOR=`cat CMakeLists.txt | match 's/set\(VERSION_MINOR.*([0-9]+).*\)/\1/p'`
    LIB_VERSION_PATCH=`cat CMakeLists.txt | match 's/set\(VERSION_PATCH.*([0-9]+).*\)/\1/p'`
    LIB_VERSION=$LIB_VERSION_MAJOR.$LIB_VERSION_MINOR.$LIB_VERSION_PATCH

    COMMIT_HASH=`echo $GITHUB_SHA | cut -c 1-6`
    LIB_VERSION="$LIB_VERSION-rc-$COMMIT_HASH"

    echo "=====> Libcore version : $LIB_VERSION"

    echo "======= Pushing to S3 =========="
    cd $out/lib
    ls -la
    aws s3 sync ./ s3://ledger-lib-ledger-core/$LIB_VERSION/ --acl public-read --exclude "*" --include "*.so" --include "*.dylib" && \
    aws s3 ls s3://ledger-lib-ledger-core/$LIB_VERSION;
    cd -
  '';
}
