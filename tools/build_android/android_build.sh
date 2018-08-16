#Root to polly toolchains
export POLLY_ROOT="/Users/elkhalilbellakrid/Desktop/Playground_15/lib-ledger-core/toolchains/polly"
export ANDROID_NDK_r16b="/Users/elkhalilbellakrid/Library/Android/sdk/ndk-bundle"
#-DCMAKE_CONFIGURATION_TYPES=Release
cmake -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_TOOLCHAIN_FILE=../lib-ledger-core/toolchains/polly/android-ndk-r16b-api-21-x86-clang-libcxx.cmake ../lib-ledger-core
cmake --build . --config Release

