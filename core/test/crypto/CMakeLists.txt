cmake_minimum_required(VERSION 3.0)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

add_executable(ledger-core-crypto-tests main.cpp digest_test.cpp encryption_test.cpp deterministic_public_key_test.cpp)

target_link_libraries(ledger-core-crypto-tests gtest gtest_main)
target_link_libraries(ledger-core-crypto-tests ledger-core)
target_link_libraries(ledger-core-crypto-tests ledger-test)
include_directories(../lib/libledger-test/)
add_test (NAME ledger-core-crypto-tests COMMAND ledger-core-crypto-tests)