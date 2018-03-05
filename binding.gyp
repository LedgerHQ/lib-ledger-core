{
  'targets': [
      {
      'target_name': 'ledgerapp_nodejs',
      'sources': [
            "<!@(python glob.py api/core/nodejs *.cpp *.hpp)"
       ],
       'include_dirs': [
			"<!(node -e \"require('nan')\")",
			"../lib-ledger-core-build/include/ledger/core",
        ],
		  'all_dependent_settings': {
			'include_dirs': [
				"<!(node -e \"require('nan')\")"
			],
		},
      'conditions': [
        [ 'OS=="mac"', {
              'LDFLAGS': [
            '-framework IOKit',
            '-framework CoreFoundation'
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
        [ 'OS=="win"', {
			'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
			'OTHER_CFLAGS': [
			  '-std=c++14',
			]
       	 }]
      ],
      'cflags!': ['-ansi', '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      'cflags': ['-g', '-exceptions'],
      'cflags_cc': ['-g', '-exceptions']
    }
  ]
}
