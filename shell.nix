{}:

let
  pkgs = import ./nix/pkgs.nix { jdk = "jdk8"; };
  secp256k1-chfast = import ./nix/secp256k1.nix { inherit pkgs; };
in
pkgs.mkShell {
  nativeBuildInputs = (pkgs.lib.attrVals [
    "pkg-config"
    "cmake"
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

    # keep this line if you use bash
    "bashInteractive"
  ] pkgs)
  ++ [ secp256k1-chfast ]
  ++ pkgs.lib.optionals pkgs.stdenv.isLinux [ pkgs.libselinux ] ;
}
