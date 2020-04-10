project(ledger-core-algorand-tests)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(gtest_force_shared_crt ON CACHE BOOL "Build gtest with shared runtime" FORCE)

add_subdirectory(lib/googletest)
add_subdirectory(lib/libledger-test)

