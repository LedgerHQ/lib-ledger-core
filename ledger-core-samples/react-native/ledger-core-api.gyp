{
	'variables': {
		'core_library%': "../../../../lib-ledger-core-build",
		'run_path%': "../../../lib-ledger-core-build/core/src",
	},
	'targets': [
		{
			'target_name': 'libledger-core-objc',
			'type': 'static_library',
			'conditions': [],
			'dependencies': [
				'deps/djinni/support-lib/support_lib.gyp:djinni_objc',
				'libledgerapp',
			],
			'sources': [
				"<!@(python glob.py ../../api/core/objc *.h *.m *.mm)",
				"<!@(python glob.py ../../api/core/src/objc *.h *.m *.mm)",
			],
            'include_dirs': [
				"<@(core_library)/include/ledger/core",
			],
			'libraries': ['-Wl,-rpath,<!(pwd)/<@(run_path)', '-L<@(core_library)/core/src', '-lledger-core'],
			'conditions': [
				['OS=="ios"', {
					'LDFLAGS': [
						'-framework IOKit',
						'-framework CoreFoundation',
						'-DYLD_LIBRARY_PATH '
					],
					'xcode_settings': {
						'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
						'OTHER_CFLAGS': [
							'-ObjC++',
							'-std=c++14',
						],
						'OTHER_LDFLAGS': [
							'-framework IOKit',
							'-framework CoreFoundation'
						],
						'SDKROOT': 'iphoneos',
						'SUPPORTED_PLATFORMS': 'iphonesimulator iphoneos',
					},
				}],
				['OS=="win"', {
					'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
					'OTHER_CFLAGS': [
						'-std=c++14',
					]
				}]
			],
			'cflags!': ['-ansi', '-fno-exceptions'],
			'cflags_cc!': ['-fno-exceptions'],
			'cflags': ['-g', '-exceptions'],
			'cflags_cc': ['-g', '-exceptions']
		},
  ],
}
