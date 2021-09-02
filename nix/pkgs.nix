# The version of pkgs to use with the correct overrides
{ jdk }:

let
  pinned = import ./pinned.nix;
  config = import ./config.nix { inherit jdk; };
  pkgs   = import pinned.nixpkgs { inherit config; };
  compilationStdenv = if pkgs.stdenv.isLinux
           then pkgs.gcc11Stdenv
           else pkgs.llvmPackages_7.stdenv;
in
pkgs // { inherit compilationStdenv; }
