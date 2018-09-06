ifndef ANDROID_NDK_HOME
ANDROID_NDK_HOME = $(abspath $(dir $(realpath $(shell which ndk-build))))
endif

all: android

clean:
	-rm -rf build/
	-rm GypAndroid.mk
	-rm android.pyc
	-rm lib-binding.target.mk
	-rm sources

copy_sources:
	mkdir sources || echo "Source directory already exists"
	cp -r ../../../../../../djinni/support-lib sources
	cp -r  ../../../../../../core/src/jni sources
	cp ../../../../../../core/src/utils/optional.* sources/utils
	cp -r  ../../../../../../core/src/api sources
    #cp ../../../../src/java/* sources
    #cp ../../../../src/java/* android/src/main/java/com/ledger/binding

GypAndroid.mk: copy_sources
	ANDROID_BUILD_TOP=$(ANDROID_NDK_HOME) PYTHONPATH=../../../../../../tools/gyp/pylib $(which ndk-build) ../../../../../../tools/gyp/gyp --depth=. -f "./android.py" -DOS=android --root-target=lib-binding -Icommon.gypi binding.gyp 

android: GypAndroid.mk
	echo "Make android target"
	
.PHONY: android
