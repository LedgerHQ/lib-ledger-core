cmake_minimum_required(VERSION 3.0)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

add_executable(ledger-core-bytes-tests main.cpp BytesReaderTests.cpp BytesWriterTests.cpp RLPTests.cpp Zarith.cpp)

target_link_libraries(ledger-core-bytes-tests gtest gtest_main)
target_link_libraries(ledger-core-bytes-tests ledger-core-bundle)

add_test (NAME ledger-core-bytes-tests COMMAND ledger-core-bytes-tests)
