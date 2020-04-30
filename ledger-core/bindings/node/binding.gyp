{
  'variables': {
    'core_library%': 'lib',
    'source_path%': 'src',
    'conditions': [
      ['OS=="win"', { 'lib_name': 'ledger-core' }],
      ['OS!="win"', { 'lib_name': 'libledger-core' }],
    ]
  },
  'targets': [{
    'target_name': 'ledger-core-node',
    'sources': ["<!@(python glob.py <@(source_path) *.cpp *.hpp)"],
    'include_dirs': [
      "<!(node -e \"require('nan')\")",
      "<(module_root_dir)/include",
    ],
    'all_dependent_settings': {
      'include_dirs': ["<!(node -e \"require('nan')\")"],
    },
    "copies": [
      {
        'files': [ '<(module_root_dir)/<(core_library)/<(lib_name)<(SHARED_LIB_SUFFIX)' ],
        'destination': '<(module_root_dir)/build/Release/',
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
          '-L<(module_root_dir)/<@(core_library)',
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
            'files': [ '<(module_root_dir)/<(core_library)/<(lib_name)<(SHARED_LIB_SUFFIX)' ],
            'destination': '<(module_root_dir)/build/Release/',
          },
          {
            'files': [ '<(module_root_dir)/<(core_library)/crypto<(SHARED_LIB_SUFFIX)' ],
            'destination': '<(module_root_dir)/build/Release/',
          },
          {
            'files': [ '<(module_root_dir)/<(core_library)/<(lib_name).lib' ],
            'destination': '<(module_root_dir)/build/',
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
          '-L<(module_root_dir)/<@(core_library)',
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
