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
  ],
}
