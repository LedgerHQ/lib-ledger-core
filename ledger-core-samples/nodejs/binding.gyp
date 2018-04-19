{
	'variables': {
		'core_library%': "../../../../lib-ledger-core-build",
		'run_path%': "../../../lib-ledger-core-build/core/src",
		'source_path%': "../../api/core/nodejs",
	},
	'targets': [{
		'target_name': 'ledgerapp_nodejs',
		'sources': [
			"<!@(python glob.py <@(source_path) *.cpp *.hpp)"
		],
		'include_dirs': [
			"<!(node -e \"require('nan')\")",
			"<@(core_library)/include/ledger/core",
		],
		'all_dependent_settings': {
			'include_dirs': [
				"<!(node -e \"require('nan')\")"
			],
		},
		'libraries': ['-Wl,-rpath,<!(pwd)/<@(run_path)', '-L<@(core_library)/core/src', '-lledger-core'],
		'conditions': [
			['OS=="mac"', {
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
	}]
}