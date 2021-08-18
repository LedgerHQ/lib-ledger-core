{ pkgs ? import <nixpkgs> {} }:

with pkgs;
stdenv.mkDerivation {
  pname = "secp256k1-chfast";

  # I can't find any version numbers, so we're just using the date of the
  # last commit.
  version = "ac8ccf29b8c6b2b793bc734661ce43d1f952977a";

  src = fetchFromGitHub {
    owner = "chfast";
    repo = "secp256k1";
    rev = "ac8ccf29b8c6b2b793bc734661ce43d1f952977a";
    sha256 = "0kq4fyvi3mz0cggbwjrl9av16jkq7cifr3ac8b7mpi3ycw4babpf";
  };

  nativeBuildInputs = [ 
    autoreconfHook
    pkg-config
  ];

  configureFlags = [
    "--enable-benchmark=no"
    "--enable-exhaustive-tests=no"
    "--enable-experimental"
    "--enable-module-ecdh"
    "--enable-module-recovery"
    "--enable-module-schnorrsig"
    "--enable-tests=yes"
    "--enable-static"
  ];

  doCheck = true;

  checkPhase = "./tests";

  meta = with lib; {
    description = "Optimized C library for EC operations on curve secp256k1";
    longDescription = ''
      Optimized C library for EC operations on curve secp256k1. Part of
      Bitcoin Core. This library is a work in progress and is being used
      to research best practices. Use at your own risk.
    '';
    homepage = "https://github.com/chfast/secp256k1";
    license = with licenses; [ mit ];
    maintainers = with maintainers; [ ];
    platforms = with platforms; unix;
  };
}
