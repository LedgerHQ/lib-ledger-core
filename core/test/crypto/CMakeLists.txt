cmake_minimum_required(VERSION 3.0)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

add_executable(ledger-core-crypto-tests main.cpp digest_test.cpp)

target_link_libraries(ledger-core-crypto-tests gtest gtest_main)
target_link_libraries(ledger-core-crypto-tests ledger-core)
add_test (NAME ledger-core-crypto-tests COMMAND ledger-core-crypto-tests)