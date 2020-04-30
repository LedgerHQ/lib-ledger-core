cmake_minimum_required(VERSION 3.0)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

add_executable(ledger-core-common-tests main.cpp BalanceHistory.cpp)

target_link_libraries(ledger-core-common-tests gtest gtest_main)
target_link_libraries(ledger-core-common-tests ledger-core-obj)
target_link_libraries(ledger-core-common-tests ledger-test)
target_link_libraries(ledger-core-common-tests leveldb)

add_test (NAME ledger-core-common-tests COMMAND ledger-core-common-tests)

