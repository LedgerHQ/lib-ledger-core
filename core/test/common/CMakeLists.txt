cmake_minimum_required(VERSION 3.0)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

add_executable(ledger-core-common-tests main.cpp balance_history.cpp)

target_link_libraries(ledger-core-common-tests gtest gtest_main)
target_link_libraries(ledger-core-common-tests ledger-core-static)
target_link_libraries(ledger-core-common-tests ledger-test)
target_link_libraries(ledger-core-common-tests leveldb)
target_include_directories(ledger-core-common-tests PUBLIC ../../../core/lib/leveldb/include)
include_directories(../lib/libledger-test/)

include(CopyAndInstallImportedTargets)
copy_install_imported_targets(ledger-core-common-tests crypto)

add_test (NAME ledger-core-common-tests COMMAND ledger-core-common-tests)

