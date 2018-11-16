{
	'variables': {
		'core_library%': "../../../../../lib-ledger-core-build",
		'run_path%': "../../../../../lib-ledger-core-build/core/src/Release-iphonesimulator",
		'header_path%': "../../objc",
	},
	'targets': [
		{
			'target_name': 'libledger-core-objc',
			'type': 'static_library',
			'conditions': [],
			'dependencies': [
				'../../../../djinni/support-lib/support_lib.gyp:djinni_objc'
			],
			'sources': [
				"<!@(python glob.py ../../objc *.h *.m *.mm)",
				"<!@(python glob.py ../../objcpp *.h *.m *.mm)",
				"<!@(python glob.py ../../src/objc *.h *.m *.mm)",
			],
            'include_dirs': [
				"<@(core_library)/include/ledger/core/api",
				"<@(header_path)",
			],
			'all_dependent_settings': {
				'include_dirs': [
				  "<@(core_library)/include/ledger/core/api",
				  "<@(header_path)",
				],
			},
			'libraries': ['<!(pwd)/<@(run_path)/libledger-core.dylib']
		},
        {
            'target_name': 'libledger-core-android',
            'android_unmangled_name': 1,
            'type': 'shared_library',
            'dependencies': [
                #'../../../../djinni/support-lib/support_lib.gyp:djinni_jni'
                '../../../../djinni/support-lib/support_lib.gyp:djinni_jni_main'
            ],
            'ldflags' : [ '-llog' ],
            'sources': [
                '<!@(python glob.py android/src/java *.java)',
            ],
            'include_dirs': [
                #"<@(core_library)/include/ledger/core/api",
                #"<!(echo %JAVA_HOME%)/include",
                #"<!(echo %ANDROID_NDK%)/sysroot/usr/include",
                #"/System/Library/Frameworks/JavaVM.framework/Versions/Current/Headers"
                "android/include",
                "android/src/jni",
                "android/src/jni/jni",
            ],
            'all_dependent_settings': {
                'include_dirs': [
                    #"<@(core_library)/include/ledger/core/api",
                    #"<!(echo %JAVA_HOME%)/include",
                    #"<!(echo %ANDROID_NDK%)/sysroot/usr/include",
                    #"/System/Library/Frameworks/JavaVM.framework/Versions/Current/Headers"
                    "android/include",
                    "android/src/jni",
                    "android/src/jni/jni",
                ],
            },
        },
  ],
}
