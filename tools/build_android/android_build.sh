#Root to polly toolchains
export POLLY_ROOT="/Users/elkhalilbellakrid/Desktop/Playground_15/lib-ledger-core/toolchains/polly"
export ANDROID_NDK_r16b="/Users/elkhalilbellakrid/Library/Android/sdk/ndk-bundle"
export ANDROID_NDK_r14="/Users/elkhalilbellakrid/Library/Android/sdk/ndk-bundle"

export JAVA_HOME="$(/usr/libexec/java_home -v 1.8)" || export JAVA_HOME="/usr/lib/jvm/java-11-openjdk-amd64/"
export JAVA_HOME="$(/usr/libexec/java_home -v 1.8)"
#export JAVA_INCLUDE_PATH2="$(echo $JAVA_HOME)"/include
#echo "************"
#echo $JAVA_HOME
#echo $JAVA_INCLUDE_PATH2
#echo "************"
#-DCMAKE_CONFIGURATION_TYPES=Release
#-DTARGET_JNI=ON
#android-ndk-r16b-api-21-x86-clang-libcxx link with .dylib
#android-ndk-r14-api-21-x86-64 complains about  std::strtoll in soci_sqlite3
#android-ndk-r16b-api-16-x86-clang-libcxx14 link with .dylib
#android-ndk-r16b-api-21-x86-64-clang-libcxx
cmake -DCMAKE_BUILD_TYPE:STRING=Release -DTARGET_JNI=ON -DCMAKE_TOOLCHAIN_FILE=../lib-ledger-core/toolchains/polly/android-ndk-r16b-api-21-x86-clang-libcxx.cmake ../lib-ledger-core

#cmake -DCMAKE_BUILD_TYPE:STRING=Release -DANDROID_OSX=ON -DTARGET_JNI=ON -DCMAKE_TOOLCHAIN_FILE=../lib-ledger-core/toolchains/polly/android-ndk-r16b-api-24-arm64-v8a-clang-libcxx14.cmake ../lib-ledger-core

cmake --build . --config Release

