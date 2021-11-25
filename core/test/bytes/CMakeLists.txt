cmake_minimum_required(VERSION 3.0)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)
include_directories(${CMAKE_BINARY_DIR}/include/ledger/core)

add_executable(ledger-core-bytes-tests main.cpp bytesreader_tests.cpp byteswriter_tests.cpp RLP_tests.cpp zarith.cpp)

target_link_libraries(ledger-core-bytes-tests gtest gtest_main)
target_link_libraries(ledger-core-bytes-tests ledger-core-static)

include(GoogleTest)
gtest_discover_tests(ledger-core-bytes-tests PROPERTIES "LABELS;unit")