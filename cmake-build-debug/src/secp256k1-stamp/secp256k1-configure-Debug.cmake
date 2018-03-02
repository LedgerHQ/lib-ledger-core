

set(command "/Applications/CLion.app/Contents/bin/cmake/bin/cmake;-DCMAKE_INSTALL_PREFIX=/Users/elkhalilbellakrid/Desktop/Playground_03/lib-ledger-core/cmake-build-debug;-DCMAKE_POSITION_INDEPENDENT_CODE=;-DCMAKE_C_COMPILER=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc;-DCMAKE_CXX_COMPILER=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++;-GCodeBlocks - Unix Makefiles;/Users/elkhalilbellakrid/Desktop/Playground_03/lib-ledger-core/cmake-build-debug/src/secp256k1")
execute_process(
  COMMAND ${command}
  RESULT_VARIABLE result
  OUTPUT_FILE "/Users/elkhalilbellakrid/Desktop/Playground_03/lib-ledger-core/cmake-build-debug/src/secp256k1-stamp/secp256k1-configure-out.log"
  ERROR_FILE "/Users/elkhalilbellakrid/Desktop/Playground_03/lib-ledger-core/cmake-build-debug/src/secp256k1-stamp/secp256k1-configure-err.log"
  )
if(result)
  set(msg "Command failed: ${result}\n")
  foreach(arg IN LISTS command)
    set(msg "${msg} '${arg}'")
  endforeach()
  set(msg "${msg}\nSee also\n  /Users/elkhalilbellakrid/Desktop/Playground_03/lib-ledger-core/cmake-build-debug/src/secp256k1-stamp/secp256k1-configure-*.log")
  message(FATAL_ERROR "${msg}")
else()
  set(msg "secp256k1 configure command succeeded.  See also /Users/elkhalilbellakrid/Desktop/Playground_03/lib-ledger-core/cmake-build-debug/src/secp256k1-stamp/secp256k1-configure-*.log")
  message(STATUS "${msg}")
endif()
