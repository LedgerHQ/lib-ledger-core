# The version of pkgs to use with the correct overrides
{ jdk }:

let
  pinned = import ./pinned.nix;
  config = import ./config.nix { inherit jdk; };
  pkgs   = import pinned.nixpkgs { inherit config; };
in
  pkgs
