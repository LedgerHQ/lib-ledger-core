{}:

let
  pkgs = import ./pkgs.nix { jdk = "jdk8"; };
  secp256k1-chfast = import ./secp256k1.nix { inherit pkgs; };
in
(pkgs.mkShell.override {stdenv = pkgs.compilationStdenv;}) {

  nativeBuildInputs = [
    pkgs.pkg-config
    pkgs.cmake
  ];
  buildInputs = [
    # Common build deps
    pkgs.postgresql_12
    pkgs.openssl_1_1
    pkgs.sqlite
    pkgs.libkrb5
    pkgs.cryptopp
    secp256k1-chfast

    # JNI bindings deps
    pkgs.jdk8

    # Test deps
    pkgs.libsForQt515.qt5.qtbase
    pkgs.libsForQt515.qt5.qtconnectivity
    pkgs.libsForQt515.qt5.qtwebsockets
    pkgs.libuv

    # keep this line if you use bash
    pkgs.bashInteractive
  ]
  ++ pkgs.lib.optionals pkgs.stdenv.isLinux [ pkgs.libselinux ];
}
