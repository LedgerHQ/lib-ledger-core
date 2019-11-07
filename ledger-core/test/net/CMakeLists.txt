cmake_minimum_required(VERSION 3.0)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${gmock_SOURCE_DIR}/include ${gmock_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)
if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

include_directories(../lib/libledger-test/)

add_executable(ledger-core-net-tests main.cpp HTTPClientTests.cpp WebSocketClientTests.cpp)

target_link_libraries(ledger-core-net-tests gtest gtest_main)
target_link_libraries(ledger-core-net-tests gmock)
target_link_libraries(ledger-core-net-tests ledger-core-static)
target_link_libraries(ledger-core-net-tests ledger-test)
target_link_libraries(ledger-core-net-tests ledger-qt-host)

target_include_directories(ledger-core-net-tests PUBLIC ../lib/qt-host)

include(CopyAndInstallImportedTargets)
copy_install_imported_targets(ledger-core-net-tests crypto)

add_test (NAME ledger-core-net-tests COMMAND ledger-core-net-tests)
