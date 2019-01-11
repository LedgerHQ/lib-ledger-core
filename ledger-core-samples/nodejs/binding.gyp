{
  'variables': {
    'binding_src%': "lib-ledger-core/api/core/nodejs",
	'core_lib_path%': "lib-ledger-core-build/core/src",
	'crypto_lib_path%': "lib-ledger-core-build/core/lib/openssl/crypto",
	'core_include%': "lib-ledger-core-build/include/ledger/core",
	'binding_proj_folder%': "lib-ledger-core/ledger-core-samples/nodejs/build",
    'ledger_root%': "../../../",
	'conditions': [
      ['OS=="win"', { 'lib_name': 'ledger-core' }],
      ['OS!="win"', { 'lib_name': 'libledger-core' }],
    ]
  },
  'targets': [{
    'target_name': 'ledger-core',
    'sources': ["<!@(python glob.py <@(ledger_root)/<@(binding_src) *.cpp *.hpp)"],
    'include_dirs': [
      "<!(node -e \"require('nan')\")",
      "<@(ledger_root)/<@(core_include)",
    ],
    'all_dependent_settings': {
      'include_dirs': ["<!(node -e \"require('nan')\")"],
    },
    "copies": [
      {
        'files': [ '<@(ledger_root)/<@(core_lib_path)/build/Release/<(lib_name)<(SHARED_LIB_SUFFIX)' ],
        'destination': '<@(ledger_root)/<@(binding_proj_folder)/Release/',
      },
    ],
    'conditions': [
      ['OS=="mac"', {
        'LDFLAGS': [
          '-framework IOKit',
          '-framework CoreFoundation',
          '-DYLD_LIBRARY_PATH '
        ],
        'libraries': [
          '-L<@(ledger_root)/<@(core_lib_path)',
          '-lledger-core'
        ],
        'xcode_settings': {
          'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
          'OTHER_CFLAGS': [
            '-ObjC++',
            '-std=c++14',
          ],
          'OTHER_LDFLAGS': [
            '-framework IOKit',
            '-framework CoreFoundation',
            '-Xlinker -rpath -Xlinker @loader_path/'
          ],
        },
      }],
      ['OS=="win"', {
        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
        'OTHER_CFLAGS': ['-std=c++14'],
        "copies": [
		  {
			'files': [ '<@(ledger_root)/<@(core_lib_path)/build/Release/<(lib_name)<(SHARED_LIB_SUFFIX)' ],
			'destination': '<@(ledger_root)/<@(binding_proj_folder)/Release/',
		  },
		  {
			'files': [ '<@(ledger_root)/<@(crypto_lib_path)/build/Release/crypto<(SHARED_LIB_SUFFIX)' ],
			'destination': '<@(ledger_root)/<@(binding_proj_folder)/Release/',
		  },
          {
            'files': [ '<@(ledger_root)/<@(core_lib_path)/Release/<(lib_name).lib' ],
            'destination': '<@(ledger_root)/<@(binding_proj_folder)/',
          },
        ],
        'include_dirs': ['$(UCRT_BASED_PATH)'],
        'libraries': ['ledger-core'],
      }],
      ['OS=="linux"', {
        'ldflags': [
          '-Wl,-R,\'$$ORIGIN\'',
        ],
        'libraries': [
          '-L<@(ledger_root)/<@(core_lib_path)',
          '-lledger-core',
        ],
      }],
    ],
    'cflags!': ['-ansi', '-fno-exceptions'],
    'cflags_cc!': ['-fno-exceptions'],
    'cflags': ['-g', '-exceptions'],
    'cflags_cc': ['-g', '-exceptions']
  }]
}
