cmake_minimum_required(VERSION 3.0)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)
if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

add_executable(ledger-core-async-tests main.cpp callback_test.cpp)

target_link_libraries(ledger-core-async-tests gtest gtest_main)
target_link_libraries(ledger-core-async-tests ledger-core)
target_link_libraries(ledger-core-async-tests ledger-test)
target_include_directories(ledger-core-async-tests PUBLIC ../lib/libledger-test)

add_test (NAME ledger-core-async-tests COMMAND ledger-core-math-tests)