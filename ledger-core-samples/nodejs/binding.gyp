{
	'variables': {
		'core_library%': "../../../../lib-ledger-core-build",
		'run_path%': "../../../lib-ledger-core-build",
		'source_path%': "../../api/core/nodejs",
	},
	'targets': [{
		'target_name': 'ledger-core',
		'sources': [
			"<!@(python glob.py <@(source_path) *.cpp *.hpp)"
		],
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
				'include_dirs': [
					"<!(node -e \"require('nan')\")",
					"<@(core_library)/include/ledger/core",
				],
				'libraries': ['-Wl,-rpath,<!(pwd)/<@(run_path)/core/src', '-L<@(core_library)/core/src', '-lledger-core'],
			}],
			['OS=="linux"', {
				'include_dirs': [
					"<!(node -e \"require('nan')\")",
					"<@(core_library)/include/ledger/core",
				],
				'libraries': ['-Wl,-rpath,<!(pwd)/<@(run_path)/core/src', '-L<@(core_library)/core/src', '-lledger-core'],
			}],
			['OS=="win"', {
				'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
				'OTHER_CFLAGS': [
					'-std=c++14',
				],
				"copies": [
					{
						'files': [ '<@(run_path)/core/src/build/Release/ledger-core<(SHARED_LIB_SUFFIX)' ],
						'destination': '<(module_root_dir)/build/Release',
					},
					{
						'files': [ '<@(run_path)/core/lib/openssl/crypto/build/Release/crypto<(SHARED_LIB_SUFFIX)' ],
						'destination': '<(module_root_dir)/build/Release',
					},
					{
						'files': [ '<@(run_path)/core/src/Release/ledger-core.lib' ],
						'destination': '<(module_root_dir)/build',
					},
				],
				'include_dirs': [
					"<!(node -e \"require('nan')\")",
					"<@(run_path)/include/ledger/core"],
				'libraries': ['ledger-core'],
			}]
		],
		'cflags!': ['-ansi', '-fno-exceptions'],
		'cflags_cc!': ['-fno-exceptions'],
		'cflags': ['-g', '-exceptions'],
		'cflags_cc': ['-g', '-exceptions']
	}]
}