#!/usr/bin/env python3

import sys

if __name__ == "__main__":
    args = sys.argv[1:]
    copies = ", ".join([f"'<(module_root_dir)/<(core_library)/<(lib_name_base)-{s}<(SHARED_LIB_SUFFIX)'" for s in args if s != "core"])
    llibs = ", ".join([f"'-lledger-core-{s}'" for s in args if s != "core"])
    libs = ", ".join([f"'ledger-core-{s}'" for s in args if s != "core"])

    with open("./tools/templates/binding.gyp.tpl", "r") as tpl_file:
        tpl = tpl_file.read()
        substituted = tpl.format(copies=copies, libs=libs, llibs=llibs)

        with open("./bindings/node/binding.gyp", "w") as output_file:
            output_file.write(substituted)
