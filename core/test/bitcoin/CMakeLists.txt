cmake_minimum_required(VERSION 3.0)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

add_executable(ledger-core-bitcoin-tests main.cpp address_test.cpp bitcoin_helper_tests.cpp script_tests.cpp)

target_link_libraries(ledger-core-bitcoin-tests gtest gtest_main)
target_link_libraries(ledger-core-bitcoin-tests ledger-core-static)
target_link_libraries(ledger-core-bitcoin-tests ledger-test)
target_link_libraries(ledger-core-bitcoin-tests leveldb)
target_include_directories(ledger-core-bitcoin-tests PUBLIC ../../../core/lib/leveldb/include)
include_directories(../lib/libledger-test/)

include(CopyAndInstallImportedTargets)
copy_install_imported_targets(ledger-core-bitcoin-tests crypto)

add_test (NAME ledger-core-bitcoin-tests COMMAND ledger-core-bitcoin-tests)