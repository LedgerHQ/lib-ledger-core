{ pkgs ? import <nixpkgs> {} }:

with pkgs;
stdenv.mkDerivation {
  pname = "ethash-ledgerhq";

  version = "unstable-20181217";

  src = fetchFromGitHub {
    owner = "gagbo";
    repo = "ethash";
    rev = "19aaccac11e05b8be6f634e68bd46dfa2c616da8";
    sha256 = "07qmdq9sanp29wbfh3sl813355hp1pjr5qcr19inlrs59hnf7710";
  };

  nativeBuildInputs = [
    cmake
  ];

  checkInputs = [
    gbenchmark
    gtest
  ];

  cmakeFlags = [
    "-DHUNTER_ENABLED=OFF"
    "-DETHASH_BUILD_TESTS=OFF"
  ];

  # NOTE: disabling tests due to gtest issue
  doCheck = false;

  meta = with lib; {
    description = "PoW algorithm for Ethereum 1.0 based on Dagger-Hashimoto";
    homepage = "https://github.com/ledgerhq/ethash";
    platforms = platforms.unix;
    maintainers = with maintainers; [ ];
    license = licenses.asl20;
  };

}
