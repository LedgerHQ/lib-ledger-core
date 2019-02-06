project(ledger_core_tests)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

cmake_policy(SET CMP0077 NEW) # disallow options to override normal variable (not cached)

set(gtest_force_shared_crt ON BOOL "Build gtest with shared runtime")
add_subdirectory(lib/googletest)

add_subdirectory(lib/libledger-test)
add_subdirectory(bytes)
add_subdirectory(math)
add_subdirectory(utils)
add_subdirectory(async)
add_subdirectory(preferences)
add_subdirectory(debug)
add_subdirectory(net)
add_subdirectory(crypto)
add_subdirectory(bitcoin)
add_subdirectory(collections)
add_subdirectory(integration)
add_subdirectory(database)
add_subdirectory(events)
add_subdirectory(parsers)