cmake_minimum_required(VERSION 3.0)
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

add_executable(ledger-core-events-tests main.cpp EventsTest.cpp)

target_link_libraries(ledger-core-events-tests gtest gtest_main)
target_link_libraries(ledger-core-events-tests ledger-core-bundle)
target_link_libraries(ledger-core-events-tests ledger-test)
target_link_libraries(ledger-core-events-tests ledger-qt-host)

add_test (NAME ledger-core-events-tests COMMAND ledger-core-events-tests)
