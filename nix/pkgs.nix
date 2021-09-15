# The version of pkgs to use with the correct overrides
{ jdk }:

let
  pinned = import ./pinned.nix;
  config = import ./config.nix { inherit jdk; };
  pkgs   = import pinned.nixpkgs { inherit config; };
  ethash-ledgerhq = import ./ethash.nix { inherit pkgs; };
  compilationStdenv = if pkgs.stdenv.isLinux
           then pkgs.gcc11Stdenv
           else pkgs.llvmPackages_12.stdenv;
in
pkgs // { inherit compilationStdenv; inherit ethash-ledgerhq; }
