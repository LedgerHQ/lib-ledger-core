{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  nativeBuildInputs = [ 
    pkgs.pkg-config
    pkgs.cmake
  ];
  buildInputs = [
    # Common build deps
    pkgs.postgresql
    pkgs.openssl_1_1
    pkgs.gcc11
    pkgs.sqlite
    pkgs.cmake
    pkgs.libselinux
    pkgs.libkrb5
    pkgs.cryptopp

    # JNI bindings deps
    pkgs.jdk8

    # Test deps
    pkgs.libsForQt515.qt5.qtbase
    pkgs.libsForQt515.qt5.qtconnectivity
    pkgs.libsForQt515.qt5.qtwebsockets

    # keep this line if you use bash
    pkgs.bashInteractive
  ];
}
