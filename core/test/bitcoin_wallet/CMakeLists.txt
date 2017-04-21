cmake_minimum_required(VERSION 3.0)
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)
if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

include_directories(../lib/libledger-test/)

add_executable(ledger-core-bitcoin-wallet-tests main.cpp initialization_test.cpp p2pkh_keychain_test.cpp ledger_api_blockchain_explorer_tests.cpp)

target_link_libraries(ledger-core-bitcoin-wallet-tests gtest gtest_main)
target_link_libraries(ledger-core-bitcoin-wallet-tests ledger-core)
target_link_libraries(ledger-core-bitcoin-wallet-tests ledger-test)
target_link_libraries(ledger-core-bitcoin-wallet-tests ledger-qt-host)
target_include_directories(ledger-core-bitcoin-wallet-tests PUBLIC ../../../qt-host)
add_test (NAME ledger-core-bitcoin-wallet-tests COMMAND ledger-core-bitcoin-wallet-tests)