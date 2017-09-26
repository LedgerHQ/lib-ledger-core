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

add_executable(ledger-core-database-tests main.cpp pool_tests.cpp query_filters_tests.cpp query_builder_tests.cpp
            BaseFixture.cpp BaseFixture.h
        )

target_link_libraries(ledger-core-database-tests gtest gtest_main)
target_link_libraries(ledger-core-database-tests ledger-core)
target_link_libraries(ledger-core-database-tests ledger-test)
target_link_libraries(ledger-core-database-tests ledger-qt-host)
target_include_directories(ledger-core-database-tests PUBLIC ../../../qt-host)
target_include_directories(ledger-core-database-tests PUBLIC ../../../core/src/)
add_test(NAME ledger-core-database-tests COMMAND ledger-core-database-tests)