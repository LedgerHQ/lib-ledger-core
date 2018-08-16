# Android makefile for libledger-core-android shared lib, jni wrapper around libledger-core-android C API

APP_ABI := all
APP_OPTIM := release
APP_PLATFORM := android-8
# GCC 4.9 Toolchain - requires NDK r10
#NDK_TOOLCHAIN_VERSION = 4.9
# GNU libc++ is the only Android STL which supports C++11 features
APP_STL := gnustl_static
APP_BUILD_SCRIPT := jni/Android.mk
APP_MODULES := libledger-core-android
