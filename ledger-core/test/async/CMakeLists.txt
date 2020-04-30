cmake_minimum_required(VERSION 3.0)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

add_executable(ledger-core-async-tests main.cpp FutureTest.cpp PromiseTest.cpp ThreadingTests.cpp)

target_link_libraries(ledger-core-async-tests gtest gtest_main)
target_link_libraries(ledger-core-async-tests ledger-core-obj)
target_link_libraries(ledger-core-async-tests ledger-test)
target_link_libraries(ledger-core-async-tests ledger-qt-host)

add_test (NAME ledger-core-async-tests COMMAND ledger-core-async-tests)
