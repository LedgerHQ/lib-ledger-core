{ jdk ? "jdk8", useLibcoreDerivation ? false }:

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

  libledger-core = import ../default.nix { jni = true; release = true; pushS3 = false; runTests = false; };
  libcore_lib_dir = if useLibcoreDerivation
                    then "${libledger-core.out}/lib/"
                    else "jar_build/src/main/resources/resources/djinni_native_libs/";
  name = "libledger-core-jar";
in
pkgs.compilationStdenv.mkDerivation {
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

      "bashInteractive"
  ] pkgs)
  ++ [ libledger-core ];

  shellHook = ''
  export LIBCORE_LIB_DIR=${libcore_lib_dir}
  '';

  doCheck = false;
}
