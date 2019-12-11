cmake_minimum_required(VERSION 3.0)
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

add_executable(ledger-core-parser-tests
    main.cpp
    TXParser.cpp
)

target_link_libraries(ledger-core-parser-tests gtest gtest_main)
target_link_libraries(ledger-core-parser-tests ledger-core-static)
target_link_libraries(ledger-core-parser-tests ledger-test)
target_link_libraries(ledger-core-parser-tests ledger-qt-host)

add_test (NAME ledger-core-parser-tests COMMAND ledger-core-parser-tests)
