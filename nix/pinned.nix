# Holds pinned versions of packages
{
  nixpkgs = fetchTarball {
    url    = "https://github.com/NixOS/nixpkgs/archive/5f746317f10f7206f1dbb8dfcfc2257b04507eee.tar.gz";
    sha256 = "sha256:1wx5g7a5bxa01pz7c5fdp5xk1gsj7dv6xwm1a17g6a8s2a2c05x3";
  };

  gitignoreSrc = pkgs: pkgs.fetchFromGitHub {
    owner = "hercules-ci";
    repo = "gitignore.nix";
    rev = "211907489e9f198594c0eb0ca9256a1949c9d412";
    sha256 = "sha256:06j7wpvj54khw0z10fjyi31kpafkr6hi1k0di13k1xp8kywvfyx8";
  };
}
