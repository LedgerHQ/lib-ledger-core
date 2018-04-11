{
	'variables': {
		'core_library%': "../../../../../../lib-ledger-core-build",
		'run_path%': "../../../../../lib-ledger-core-build/core/src",
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
				"<!@(python glob.py ../../src/objc *.h *.m *.mm)",
			],
            'include_dirs': [
				"<@(core_library)/include/ledger/core",
				"<@(header_path)",
			],
			"direct_dependent_settings": {
                    "include_dirs": [
                      "<@(header_path)",
                    ],
            },
			'libraries': ['<!(pwd)/<@(run_path)/libledger-core.a'],
			'conditions': [
				['OS=="ios"', {
					'LDFLAGS': [
						'-framework IOKit',
						'-framework CoreFoundation',
					],
					'xcode_settings': {
						'SDKROOT': 'iphoneos',
						'SUPPORTED_PLATFORMS': 'iphonesimulator iphoneos',
						'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
						'CLANG_ENABLE_OBJC_ARC': 'YES',
						'OTHER_LDFLAGS': [
							'-framework IOKit',
							'-framework CoreFoundation'
						],
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
