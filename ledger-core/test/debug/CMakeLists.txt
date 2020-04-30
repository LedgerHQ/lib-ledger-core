cmake_minimum_required(VERSION 3.0)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

add_executable(ledger-core-debug-tests main.cpp LoggerTest.cpp)

target_link_libraries(ledger-core-debug-tests gtest gtest_main)
target_link_libraries(ledger-core-debug-tests ledger-core-obj)
target_link_libraries(ledger-core-debug-tests ledger-test)

add_test (NAME ledger-core-debug-tests COMMAND ledger-core-debug-tests)
