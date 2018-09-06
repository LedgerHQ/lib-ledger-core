#WARNING: for iphonesimulator build don't forget to remove FORCE in iphone.cmake (polly toolchain)
#ID used for bundle
export POLLY_IOS_BUNDLE_IDENTIFIER="com.ledger.ledgertestapp"
#Root to polly toolchains
export POLLY_ROOT="/Users/elkhalilbellakrid/Desktop/Playground_15/lib-ledger-core/toolchains/polly"
#Needed for nocodesign toolchains
export XCODE_XCCONFIG_FILE=$POLLY_ROOT/scripts/NoCodeSign.xcconfig
#cmake -GXcode -DCMAKE_OSX_ARCHITECTURES:STRING="armv7" -DCMAKE_MACOSX_BUNDLE:BOOL=ON -DCMAKE_OSX_SYSROOT:STRING="iphoneos" -DCMAKE_TOOLCHAIN_FILE=../lib-ledger-core/toolchains/polly/ios-nocodesign-11-2-dep-9-3-arm64-armv7.cmake -DCMAKE_INSTALL_PREFIX=/usr/local/Cellar/qt/5.10.0_1 ../lib-ledger-core
#iOS simulator
cmake -GXcode -DCMAKE_OSX_ARCHITECTURES:STRING="x86_64" -DCMAKE_MACOSX_BUNDLE:BOOL=ON -DCMAKE_OSX_SYSROOT:STRING="iphonesimulator" -DCMAKE_TOOLCHAIN_FILE=../lib-ledger-core/toolchains/polly/ios-nocodesign-11-2-dep-9-3.cmake -DBUILD_TESTS=OFF ../lib-ledger-core
xcodebuild -project ledger-core.xcodeproj -configuration Release -jobs 4
