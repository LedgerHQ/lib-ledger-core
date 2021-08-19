{ pkgs ? import (fetchTarball "https://github.com/NixOS/nixpkgs/archive/5f746317f10f7206f1dbb8dfcfc2257b04507eee.tar.gz") {}
}:

with pkgs;
let
  secp256k1-chfast = import ./nix/secp256k1.nix { inherit pkgs; };
in
mkShell {
  nativeBuildInputs = [ 
    pkg-config
    cmake
  ];
  buildInputs = [
    # Common build deps
    postgresql_12
    openssl_1_1
    gcc10
    sqlite
    cmake
    libkrb5
    cryptopp
    secp256k1-chfast

    # JNI bindings deps
    jdk8

    # Test deps
    libsForQt515.qt5.qtbase
    libsForQt515.qt5.qtconnectivity
    libsForQt515.qt5.qtwebsockets

    # keep this line if you use bash
    bashInteractive
  ]
  ++ lib.optionals stdenv.isLinux [ libselinux ] ;
}
