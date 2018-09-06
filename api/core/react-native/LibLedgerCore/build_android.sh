#Build JAVA binding project
cd android/binding
./build.sh || (echo "Failed to build android (java) binding" && exit)
#Build React Native bridge project
cd ..
rm -rf build libs obj || echo "LibLedgerCore/android already clean !"
./gradlew clean && ./gradlew nativeLibsToJar || echo "Failed to build React Native Bridge"
