cmake_minimum_required(VERSION 3.0)
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

file(GLOB_RECURSE TEST_FILES *.cpp)
add_executable(ledger-core-integration-tests
        ${TEST_FILES}
        ../database/IntegrationEnvironment.hpp
        )

target_link_libraries(ledger-core-integration-tests gtest gtest_main)
target_link_libraries(ledger-core-integration-tests crypto)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # We need this otherwise link fails (for Linux),
    # this is needed since we build static library for tests,
    # not needed if we link against dynamic
    # TODO: needs more investigation
    target_link_libraries(ledger-core-integration-tests ledger-core)
else()
    target_link_libraries(ledger-core-integration-tests ledger-core-static)
endif()


target_link_libraries(ledger-core-integration-tests ledger-test)
target_link_libraries(ledger-core-integration-tests ledger-qt-host)
