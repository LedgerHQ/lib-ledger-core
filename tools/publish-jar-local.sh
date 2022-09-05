set -euo pipefail

if [[ -f "$HOME/.jabba/jabba.sh" ]]; then
  echo "Found jabba"
  source "$HOME/.jabba/jabba.sh"
  jabba use adopt@1.8.0-292
fi

JAVA_VERSION=`java -version 2>&1 | awk -F '"' '{print $2}'`
if [[ $JAVA_VERSION != "1.8.0_292" ]]; then
  echo "Wrong java version ! Required 1.8.0_292 but found $JAVA_VERSION"
  exit 42
fi

JAR_BUILD_DIR=/tmp/build-jar
LIB_BUILD_DIR=$1
JAVA_API_DIR=api/core/java
SCALA_API_DIR=api/core/scala
SBT_DIR=nix/scripts/sbt
RESOURCE_DIR=$JAR_BUILD_DIR/src/main/resources/resources/djinni_native_libs

rm -rf $JAR_BUILD_DIR
mkdir -v $JAR_BUILD_DIR

mkdir -v -p $RESOURCE_DIR

cp $JAVA_API_DIR/* $JAR_BUILD_DIR
cp $SCALA_API_DIR/* $JAR_BUILD_DIR
cp -r $SBT_DIR/* $JAR_BUILD_DIR

cp $LIB_BUILD_DIR/core/src/libledger-core.so $RESOURCE_DIR

cd $JAR_BUILD_DIR
export GITHUB_TOKEN="unused" # but mandatory

export JAR_VERSION=${2-local}-SNAPSHOT

sbt publishLocal
