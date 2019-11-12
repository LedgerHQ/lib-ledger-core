{
  "variables": {
    "target_module_name" : "ledger-core",
    "lib_core_bin_path" : "../../../lib-ledger-core-build/core/src",
    "lib_core_cp_path" : "../../lib-ledger-core-build/core/src",
    "conditions": [
    	['OS=="mac"', {"wrapper_libs": "<(lib_core_bin_path)/libledger-core.dylib"}],
        ['OS=="win"', {"wrapper_libs": "<(lib_core_bin_path)/ledger-core.lib"}],
        ['OS=="linux"',{"wrapper_libs": "<(lib_core_bin_path)/libledger-core.so"}]
    ],
  },
  "includes": ["../core/lib/ubinder/src/node/ubinder.gypi"],
  "targets":
  [
	{
		"target_name": "copy_binary",
		"type":"none",
		"copies":
		[
			{
				'destination': '<(module_root_dir)/build/Release/',
				'files': [ '<(lib_core_cp_path)/libledger-core.dylib' ]
			}
        ]
	}
  ]
}
