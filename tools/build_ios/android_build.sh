#Root to polly toolchains
export POLLY_ROOT="/Users/elkhalilbellakrid/Desktop/Playground_15/lib-ledger-core/toolchains/polly"
export ANDROID_NDK_r16b="/Users/elkhalilbellakrid/Library/Android/sdk/ndk-bundle"
cmake -DCMAKE_TOOLCHAIN_FILE=../lib-ledger-core/toolchains/polly/android-ndk-r16b-api-21-x86-clang-libcxx.cmake -DBUILD_TESTS=OFF -DIS_ANDROID:BOOL=ON ../lib-ledger-core
cmake --build .

