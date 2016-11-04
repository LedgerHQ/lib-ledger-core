cmake_minimum_required(VERSION 3.0)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)
if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

add_executable(ledger-core-math-tests main.cpp bigint_tests.cpp bigint_api_tests.cpp)

target_link_libraries(ledger-core-math-tests gtest gtest_main)
target_link_libraries(ledger-core-math-tests ledger-core)
add_test (NAME ledger-core-math-tests COMMAND ledger-core-math-tests)