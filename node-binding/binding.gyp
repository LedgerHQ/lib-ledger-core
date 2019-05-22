{
  "variables": {
    "target_module_name" : "lib-core-node",
    "lib_core_bin_path" : "C:/ledger/build-2017/core/src/Debug/",
    "conditions": [
        ['OS=="win"', {"wrapper_libs": "<(lib_core_bin_path)/ledger-core.lib"}],
        ['OS=="linux"',{"wrapper_libs": "<(lib_core_bin_path)/ledger-core.so"}]
    ],
  },
  "includes": ["../core/lib/ubinder/src/node/ubinder.gypi"],
}
