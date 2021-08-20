# Config returns overrides of packages for this build
{ jdk }:

{
  # Use the given java version with sbt instead of the latest
  packageOverrides = p: {
    sbt = p.sbt.override {
      jre = p.${jdk};
    };
  };
}
