cmake_minimum_required(VERSION 3.4)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

add_executable(ledger-core-tests main.cpp)

target_link_libraries(ledger-core-tests gtest gtest_main)
target_link_libraries(ledger-core-tests ledger-core)