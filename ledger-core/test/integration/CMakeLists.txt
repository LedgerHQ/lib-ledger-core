cmake_minimum_required(VERSION 3.0)
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

file(GLOB_RECURSE TEST_FILES *.cpp)
add_library(ledger-core-integration-test
    ${TEST_FILES}
)

target_include_directories(
    ledger-core-integration-test PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> $<INSTALL_INTERFACE:include/ledger-test>
)

target_link_libraries(ledger-core-integration-test gtest gtest_main)
target_link_libraries(ledger-core-integration-test crypto)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # We need this otherwise link fails (for Linux),
    # this is needed since we build static library for tests,
    # not needed if we link against dynamic
    # TODO: needs more investigation
    target_link_libraries(ledger-core-integration-test ledger-core)
else()
    target_link_libraries(ledger-core-integration-test ledger-core-static)
endif()

target_link_libraries(ledger-core-integration-test ledger-test)
target_link_libraries(ledger-core-integration-test ledger-qt-host)
