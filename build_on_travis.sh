#!/usr/bin/env bash

# Create distribution structure

mkdir dist

# Build and test on the current OS
mkdir build && cd build && cmake .. && make && make test && cd ..

if [ "$TRAVIS_OS_NAME" == "linux" ]; then
    # Build for Windows x86
    mkdir build_win && cd build_win && cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchains/Toolchain-mingw64.cmake && make && cd ..;

    # Build for Windows x64

    # Build for Linux x86

    # Build for Linux x64

elif [ "$TRAVIS_OS_NAME" == "osx" ]; then
    # Prepare build for Android
    mkdir android_build
    cd android_build/
    mkdir ../dist/android

    # Build for Android armeabi
    rm -rf *
    cmake .. -DCMAKE_TOOLCHAIN_FILE=~/android-ndk-r13b/build/cmake/android.toolchain.cmake -DANDROID_STL_FORCE_FEATURES=true -DANDROID_ABI="armeabi" -DTARGET_JNI=true && make
    mkdir ../dist/android/armeabi; cp core/src/libledger-core.so ../dist/android/armeabi/

    # Build for Android armeabi-v7a
    rm -rf *
    cmake .. -DCMAKE_TOOLCHAIN_FILE=~/android-ndk-r13b/build/cmake/android.toolchain.cmake -DANDROID_STL_FORCE_FEATURES=true -DANDROID_ABI="armeabi-v7a" -DTARGET_JNI=true && make
    mkdir ../dist/android/armeabi-v7a; cp core/src/libledger-core.so ../dist/android/armeabi-v7a/

    # Build for Android arm64-v8a
    rm -rf *
    cmake .. -DCMAKE_TOOLCHAIN_FILE=~/android-ndk-r13b/build/cmake/android.toolchain.cmake -DANDROID_STL_FORCE_FEATURES=true -DANDROID_ABI="arm64-v8a" -DTARGET_JNI=true && make
    mkdir ../dist/android/arm64-v8a; cp core/src/libledger-core.so ../dist/android/arm64-v8a/

    # Build for Android x86_64
    rm -rf *
    cmake .. -DCMAKE_TOOLCHAIN_FILE=~/android-ndk-r13b/build/cmake/android.toolchain.cmake -DANDROID_STL_FORCE_FEATURES=true -DANDROID_ABI="x86_64" -DTARGET_JNI=true && make
    mkdir ../dist/android/x86_64; cp core/src/libledger-core.so ../dist/android/x86_64/

    # Build for Android x86
    rm -rf *
    cmake .. -DCMAKE_TOOLCHAIN_FILE=~/android-ndk-r13b/build/cmake/android.toolchain.cmake -DANDROID_STL_FORCE_FEATURES=true -DANDROID_ABI="x86" -DTARGET_JNI=true && make
    mkdir ../dist/android/x86; cp core/src/libledger-core.so ../dist/android/x86/

    # Build for Android mips
    rm -rf *
    cmake .. -DCMAKE_TOOLCHAIN_FILE=~/android-ndk-r13b/build/cmake/android.toolchain.cmake -DANDROID_STL_FORCE_FEATURES=true -DANDROID_ABI="mips" -DTARGET_JNI=true && make
    mkdir ../dist/android/mips; cp core/src/libledger-core.so ../dist/android/mips/

    # Build for Android mips64
    rm -rf *
    cmake .. -DCMAKE_TOOLCHAIN_FILE=~/android-ndk-r13b/build/cmake/android.toolchain.cmake -DANDROID_STL_FORCE_FEATURES=true -DANDROID_ABI="mips64" -DTARGET_JNI=true && make
    mkdir ../dist/android/mips64; cp core/src/libledger-core.so ../dist/android/mips64/

    # Reset state
    cd ..

    # Build for iOS

    # Build OSX
fi
