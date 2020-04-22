- RFC name: `0003-modularized-react-native-packaging`.
- Status: `draft`.
- Author: [Dimitri Sabadie](https://github.com/phaazon)
- Date Created: 2020/04/21
- Date Updated: 2020/04/21
- Summary: **N:1 React Native packaging of the modularized core (N coins, 1 React Native package).**

<!-- vim-markdown-toc GFM -->

* [Motivation](#motivation)
* [Content](#content)
  * [How packaging is done on the legacy path (analysis)](#how-packaging-is-done-on-the-legacy-path-analysis)
  * [How packaging is done on the modularized path](#how-packaging-is-done-on-the-modularized-path)
* [Rationale.](#rationale)
* [Related work](#related-work)

<!-- vim-markdown-toc -->

# Motivation
> Why such a PoC?

This is the second part of the “Ledger Live” PoC integration. After having added support for the
Node.js (Desktop) support, it was just a matter of time before starting the support of the mobile
part, which uses React Native.

The difficulty here lies in the fact that we need to configure and port two projects: the React
Native Android part, and the React Native iOS part. Because that, at the time of writing this very
document, the world is going through the Covid-19 pandemic, and the author of this RFC and
implementation (Dimitri Sabadie) doesn’t have access to a macOS system, Android will be the first
candidate for a port. However, besides some very specific implementation details for each platform,
the code between Android and iOS should remain the same, at least for the Ledger Live team.

# Content
> Content of the PoC with comments and step-by-step procedure.

## How packaging is done on the legacy path (analysis)

The script `tools/generateBindingsRN.sh` generates all the code for React Native, Java (Android),
Obj-C and Obj-C++ (iOS) inside the binding destination (typically,
`../lib-ledger-core-react-native-bindings`).

Android libraries go into the `android/libs` folder, and the iOS libraries into
the `ios/Frameworks` folder.

Then, each project is configured via different files regarding the platform:

- Android is configured via the file `android/build.gradle`.
- iOS is configured via the XCode projects found in `ios`.

From now on, we focus on Android. The `build.gradle` file contains folders only, even for the
libraries to use. So the Core libraries (i.e. `libledger-core.so`) are picked automatically
from that folder (`jniLibs.srcDirs = ['libs']`).

The `android/src` directory contains the source used to build the ReactNative library for Android.
`android/src/main/java` contains all the source files and `android/src/main/AndroidManifest.xml`
is a file containing the basic React Native manifest file. That file doesn’t contain much and
shouldn’t change that much.

Then, in `android/src/main/java`, we have `co/ledger` and `com/ledger`. The difference between those
two folders is that the `com/ledger/reactnative` folder contains all React Native (RCT types)
as well as the implementations required to be present for the Core interface (such as HTTP
clients, random number generators, etc.). `co/ledger/core` contains generated code for Java
from the Core generation process, which implies `djinni`.

Most of the work will be performed by the Android toolchain (Android Studio / whatever you are
using).

## How packaging is done on the modularized path

Because we have several coins, we need to be able to call the `djinni` React Native generator
several times (as we did with the [node packaging RFC](./0002-modularized-node-packaging.md)).
The changes required to do that

# Rationale.
> Should we go for it? Drop it?

# Related work
> What else has been done and is similar?

```
ANDROID_NDK_r16b=~/Android/Sdk/ndk/16.1.4479499 cmake .. -DCMAKE_INSTALL_PREFIX=/tmp/js-core-install -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=OFF -DTARGET_JNI=ON -DCMAKE_TOOLCHAIN_FILE=../../toolchains/polly/android-ndk-r16b-api-21-arm64-v8a-neon-clang-libcxx14.cmake
```
