{ jdk ? "jdk8" }:

let
  pkgs = import ./pkgs.nix { inherit jdk; };
  pinned = import ./pinned.nix;
  inherit (import (pinned.gitignoreSrc { inherit (pkgs) fetchFromGitHub; }) { inherit (pkgs) lib; }) gitignoreFilter;

  # the `build/` in djinni/.gitignore gets badly interpreted by gitignore, so we rebuild a
  # proper filter.
  gitIgnoreWithDjinniBuildFilter = src:
    let
      # IMPORTANT: use a let binding like this to memoize info about the git directories.
      srcIgnored = gitignoreFilter src;
    in
      path: type:
         srcIgnored path type
           || builtins.baseNameOf path == "djinni/src/build";

  libledger-core = import ../default.nix { inherit pkgs; jni = true; release = true; pushS3 = false; runTests = false; };
  name = "libledger-core-jar";
in
pkgs.stdenv.mkDerivation {
  inherit name;
  version = "4.1.1";
  src = pkgs.lib.cleanSourceWith {
      filter = gitIgnoreWithDjinniBuildFilter ../.;
      src = ../.;
      name = name + "-source";
    };

  buildInputs = (pkgs.lib.attrVals [
      "coursier"
      "${jdk}"
      "sbt"

      # Djinni command dependencies
      "gcc11"
      "bashInteractive"
  ] pkgs)
  ++ [ libledger-core ];

  shellHook = ''
  export LIBCORE_LIB=${libledger-core.out}/lib/${if pkgs.stdenv.isLinux then "*.so" else "*.dylib"}
  '';

  doCheck = false;
}
