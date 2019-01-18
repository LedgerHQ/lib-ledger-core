cmake_minimum_required(VERSION 3.0)
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)
if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

include_directories(../lib/libledger-test/)

file(GLOB ledger-core-fixtures-srcs
        "../fixtures/*.cpp"
        "../fixtures/*.h"
        )

file(GLOB_RECURSE TEST_FILES *.cpp)
add_executable(ledger-core-integration-tests
        ${ledger-core-fixtures-srcs}
        ${TEST_FILES}
        IntegrationEnvironment.h
        )

target_link_libraries(ledger-core-integration-tests gtest gtest_main)
target_link_libraries(ledger-core-integration-tests ledger-core-static)
target_link_libraries(ledger-core-integration-tests ledger-test)
target_link_libraries(ledger-core-integration-tests ledger-qt-host)
target_include_directories(ledger-core-integration-tests PUBLIC ../../../core/src)
target_include_directories(ledger-core-integration-tests PUBLIC ../../../qt-host)

include(CopyAndInstallImportedTargets)
copy_install_imported_targets(ledger-core-integration-tests crypto)

add_test (NAME ledger-core-integration-tests COMMAND ledger-core-integration-tests)