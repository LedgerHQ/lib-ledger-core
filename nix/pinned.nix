# Holds pinned versions of packages
{
  nixpkgs = fetchTarball {
    url    = "https://github.com/NixOS/nixpkgs/archive/7e9b0dff974c89e070da1ad85713ff3c20b0ca97.tar.gz";
    sha256 = "sha256:1ckzhh24mgz6jd1xhfgx0i9mijk6xjqxwsshnvq789xsavrmsc36";
  };

  gitignoreSrc = {fetchFromGitHub}: fetchFromGitHub {
    owner = "hercules-ci";
    repo = "gitignore.nix";
    rev = "211907489e9f198594c0eb0ca9256a1949c9d412";
    sha256 = "sha256:06j7wpvj54khw0z10fjyi31kpafkr6hi1k0di13k1xp8kywvfyx8";
  };
}
