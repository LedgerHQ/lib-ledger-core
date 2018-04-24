all: ios

clean:
	-rm -rf build/
	-rm -rf build_ios/

build_ios/ledger-core-objc.xcodeproj: ledger-core-objc.gyp
	PYTHONPATH=../../../../tools/gyp/pylib ../../../../tools/gyp/gyp ledger-core-objc.gyp -DOS=ios --depth=. -f xcode --generator-output=./build_ios -Icommon.gypi

xb-prettifier := $(shell command -v xcpretty >/dev/null 2>&1 && echo "xcpretty -c" || echo "cat")

ios: build_ios/ledger-core-objc.xcodeproj
	xcodebuild -project build_ios/ledger-core-objc.xcodeproj -configuration Release -target libledger-core-objc | ${xb-prettifier}

cleanup_gyp: ../../../../tools/gyp ledger-core-objc.gyp
	../../../../tools/gyp/tools/pretty_gyp.py ledger-core-objc.gyp > ledger-core-objc_tmp.gyp && mv ledger-core-objc_temp.gyp ledger-core-objc.gyp
