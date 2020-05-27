cmake_minimum_required(VERSION 3.0)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

add_executable(ledger-core-preferences-tests main.cpp PreferencesTest.cpp)

target_link_libraries(ledger-core-preferences-tests gtest gtest_main)
target_link_libraries(ledger-core-preferences-tests ledger-core-bundle)
target_link_libraries(ledger-core-preferences-tests ledger-test)

add_test (NAME ledger-core-preferences-tests COMMAND ledger-core-preferences-tests)
