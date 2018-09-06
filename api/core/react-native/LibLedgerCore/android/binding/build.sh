make clean && make
cd android
rm -rf build libs obj || echo "binding/android already clean !"
./gradlew clean && ./gradlew nativeLibsToJar 
