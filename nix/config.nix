# Config returns overrides of packages for this build
{ jdk }:

{
  # Use the given java version with sbt instead of the latest
  packageOverrides = p: {
    sbt = p.sbt.override {
      jre = p.${jdk};
    };
    ethash = p.ethash.overrideAttrs (old: {
      version = "unstable-20181217";
      src = p.fetchFromGitHub {
        owner = "ledgerhq";
        repo = "ethash";
        rev = "b21016f06da6bbce2931d2566b66b1cd3224aa80";
        sha256 = "010sm9kycsgn927mirzdmw4valq5ii6j3kn2c0s7sx450v29i0l2";
      };

    });
  };
}
