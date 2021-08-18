{ pkgs ? import <nixpkgs> {}, release ? true, jni ? true, pushS3 ? false, runTests ? false }:

with pkgs;
let
  gitignoreSrc = fetchFromGitHub {
    owner = "hercules-ci";
    repo = "gitignore.nix";
    rev = "211907489e9f198594c0eb0ca9256a1949c9d412";
    sha256 = "sha256:06j7wpvj54khw0z10fjyi31kpafkr6hi1k0di13k1xp8kywvfyx8";
  };
  inherit (import gitignoreSrc { inherit (pkgs) lib; }) gitignoreSource;
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
  jniFlag = [ (lib.optionalString jni "-DTARGET_JNI=ON") ];
in

stdenv.mkDerivation {
  name = "libledger-core";
  version = "4.1.1";
  src = gitignoreSource ./.;

  nativeBuildInputs = [
    pkg-config
    cmake
    aws
  ];

  buildInputs = [
    # Common build deps
    postgresql_12
    openssl_1_1
    gcc11
    sqlite
    cmake
    libkrb5
    cryptopp
    secp256k1-chfast

    # JNI bindings deps
    jdk8
  ]
  ++ lib.optionals stdenv.isLinux [ libselinux ] ;

  checkInputs = [
    libsForQt515.qt5.qtbase
    libsForQt515.qt5.qtconnectivity
    libsForQt515.qt5.qtwebsockets
    libsForQt515.qt5.wrapQtAppsHook
  ];

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
