ifndef ANDROID_NDK_HOME
ANDROID_NDK_HOME = $(abspath $(dir $(realpath $(shell which ndk-build))))
endif

all: ios

clean:
	-rm -rf build/
	-rm -rf build_ios/
	-rm GypAndroid.mk
	-rm android.pyc
	-rm libledger-core-android.target.mk

build_ios/ledger-core-objc.xcodeproj: ledger-core-objc.gyp
	PYTHONPATH=../../../../tools/gyp/pylib ../../../../tools/gyp/gyp ledger-core-objc.gyp -DOS=ios --depth=. -f xcode --generator-output=./build_ios -Icommon.gypi

xb-prettifier := $(shell command -v xcpretty >/dev/null 2>&1 && echo "xcpretty -c" || echo "cat")

ios: build_ios/ledger-core-objc.xcodeproj
	xcodebuild -project build_ios/ledger-core-objc.xcodeproj -configuration Release -target libledger-core-objc | ${xb-prettifier}

#djinni:
#	../../../../djinni/gradle/gradlew djinni

GypAndroid.mk: 
	ANDROID_BUILD_TOP=$(ANDROID_NDK_HOME) PYTHONPATH=../../../../tools/gyp/pylib $(which ndk-build) ../../../../tools/gyp/gyp --depth=. -f "./android.py" -DOS=android --root-target=libledger-core-android -Icommon.gypi binding.gyp 

android: GypAndroid.mk
	echo "Make android target"
	

cleanup_gyp: ../../../../tools/gyp ledger-core-objc.gyp
	../../../../tools/gyp/tools/pretty_gyp.py ledger-core-objc.gyp > ledger-core-objc_tmp.gyp && mv ledger-core-objc_temp.gyp ledger-core-objc.gyp


.PHONY: android ios