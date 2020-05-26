cmake_minimum_required(VERSION 3.0)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

add_executable(ledger-core-algorand-tests
    main.cpp
    MsgpackHelpersTests.cpp
    AlgorandAddressTests.cpp
    AlgorandExplorerTests.cpp
)

target_link_libraries(ledger-core-algorand-tests gtest gtest_main)
target_link_libraries(ledger-core-algorand-tests ledger-core-static)
target_link_libraries(ledger-core-algorand-tests ledger-test)
target_link_libraries(ledger-core-algorand-tests leveldb)
target_include_directories(ledger-core-algorand-tests PUBLIC ../../../core/lib/leveldb/include)
include_directories(../lib/libledger-test/)

target_include_directories(ledger-core-algorand-tests PRIVATE ${CMAKE_SOURCE_DIR}/src/thirdparty/msgpack-c/include)

include(CopyAndInstallImportedTargets)
copy_install_imported_targets(ledger-core-algorand-tests crypto ledger-core)

add_test (NAME ledger-core-algorand-tests COMMAND ledger-core-algorand-tests)

