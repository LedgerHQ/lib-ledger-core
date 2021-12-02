cmake_minimum_required(VERSION 3.0)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)

add_executable(ledger-core-async-tests main.cpp future_test.cpp promise_test.cpp threading_tests.cpp)

target_link_libraries(ledger-core-async-tests gtest gtest_main)
target_link_libraries(ledger-core-async-tests ledger-core-static)
target_link_libraries(ledger-core-async-tests ledger-test)
target_include_directories(ledger-core-async-tests PUBLIC ../../../qt-host)
target_include_directories(ledger-core-async-tests PUBLIC ../lib/libledger-test)
if (SYS_LIBUV)
    copy_install_imported_targets(ledger-core-async-tests PkgConfig::UV)
else()
    copy_install_imported_targets(ledger-core-async-tests uv)
endif()

include(GoogleTest)
gtest_discover_tests(ledger-core-async-tests PROPERTIES "LABELS;unit")