cmake_minimum_required(VERSION 3.0)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

add_executable(ledger-core-net-tests main.cpp HTTPClientTests.cpp WebSocketClientTests.cpp)

target_link_libraries(ledger-core-net-tests gtest gtest_main)
target_link_libraries(ledger-core-net-tests gmock)
target_link_libraries(ledger-core-net-tests ledger-core-obj)
target_link_libraries(ledger-core-net-tests ledger-test)
target_link_libraries(ledger-core-net-tests ledger-qt-host)

add_test (NAME ledger-core-net-tests COMMAND ledger-core-net-tests)
