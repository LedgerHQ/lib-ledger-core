#!/usr/bin/env bash
set -e

printf "\n============ Generate Java/Scala interface files\n"
bash tools/generate_interfaces.sh
printf "\n============ Moving native libraries (from ${LIBCORE_LIB_DIR})\n"
ls -la ${LIBCORE_LIB_DIR}
mkdir -p jar_build/src/main/resources/resources/djinni_native_libs
# We allow the cp to fail with '||:'. For local builds, only one of .so or .dylib will exist,
# and we do not want the cp to break execution
cp -v ${LIBCORE_LIB_DIR}*.{so,dylib} jar_build/src/main/resources/resources/djinni_native_libs || :
printf "\n============ Moving interface files\n"
cp -v ./api/core/java/* jar_build
cp -v ./api/core/scala/* jar_build
cp -rv ./nix/scripts/sbt/* jar_build
printf "============ Checking libs in djinni native libs dir\n"
ls -la jar_build/src/main/resources/resources/djinni_native_libs
printf "\n============ Packaging JAR (with ${LIBCORE_LIB_DIR})\n"
cd jar_build
sbt package
if [[ "${GITHUB_EVENT_NAME:-NO}" == "pull_request" ]]; then
    printf "Github event matched the \"pull_request\" type\n"
    printf "No upload being done, cause this is a pull request from a forked repository\n"
else
    printf "Running SBT publish\n"
    sbt publish
fi
printf "\n============ Showing target build, hopefully with a JAR to rename ledger-lib-core.jar\n"
mkdir -p artifact
mv target/scala-2.12/*.jar artifact/ledger-lib-core.jar
ls -la artifact

