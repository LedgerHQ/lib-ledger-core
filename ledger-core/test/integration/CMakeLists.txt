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

add_library(Core::ledger-core-integration-test ALIAS ledger-core-integration-test)

target_include_directories(
    ledger-core-integration-test PUBLIC
    # hack for centralized build
    # including parent directory allows header inclusions such as "<integration/...>"
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}> $<INSTALL_INTERFACE:include/ledger-test>
)

target_link_libraries(ledger-core-integration-test gtest gtest_main)

if(SYS_OPENSSL)
    find_package(OpenSSL)
    target_link_libraries(ledger-core-integration-test OpenSSL::crypto)
else()
    target_link_libraries(ledger-core-integration-test crypto)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # We need this otherwise link fails (for Linux),
    # this is needed since we build static library for tests,
    # not needed if we link against dynamic
    # TODO: needs more investigation
    target_link_libraries(ledger-core-integration-test ledger-core)
else()
    target_link_libraries(ledger-core-integration-test ledger-core-obj)
endif()

target_link_libraries(ledger-core-integration-test ledger-test)
target_link_libraries(ledger-core-integration-test ledger-qt-host)
