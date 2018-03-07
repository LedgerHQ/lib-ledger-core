{

	'variables': {
    		'core_library%': "../../lib-ledger-core-build",
    		'core_library0%': "../../../lib-ledger-core-build",
    		'core_library1%': "../",
    		'core_library2%': "../../",
    		'core_library3%': "../../../",
    		'core_library4%': "/Users/elkhalilbellakrid/Desktop/Playground_04/lib-ledger-core-build/core/src",
    		'core_library5%': "./build/Release/",
    		'core_library5%': "./build/Release/",
    	  },
  'targets': [
      {
      'target_name': 'ledgerapp_nodejs',
      'sources': [
            "<!@(python glob.py api/core/nodejs *.cpp *.hpp)"
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
		'library_dirs': [
			'<@(core_library)/core/src',
		],
        'libraries': ['libledger-core.dylib',],
      	'ldflags': [
      	'-Wl,-rpath,<!(pwd)',
      	'-Wl,-rpath,<@(core_library)',
      	'-Wl,-rpath,<@(core_library1)',
      	'-Wl,-rpath,<@(core_library2)',
      	'-Wl,-rpath,<@(core_library3)',
      	'-Wl,-rpath,<@(core_library5)',
      	'-Wl,-rpath,<@(core_library5)',
      	'-Wl,-rpath,<@(core_library5)',
      	],
      'conditions': [
        [ 'OS=="mac"', {
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

