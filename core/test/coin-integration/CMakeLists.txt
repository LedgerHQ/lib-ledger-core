
set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)
if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

include_directories(../lib/libledger-test/)

file(GLOB_RECURSE ledger-core-coin-integration-srcs
        "*.cpp"
        "*.h"
        "*.hpp"
        )

add_executable(ledger-core-coin-integration-tests ${ledger-core-coin-integration-srcs} ../integration/IntegrationEnvironment.cpp)
if (SYS_OPENSSL)
    find_package(OpenSSL)
    target_link_libraries(ledger-core-coin-integration-tests OpenSSL::Crypto)
else()
    target_link_libraries(ledger-core-coin-integration-tests crypto)
endif()
target_link_libraries(ledger-core-coin-integration-tests gtest gtest_main)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # We need this otherwise link fails (for Linux),
    # this is needed since we build static library for tests,
    # not needed if we link against dynamic
    # TODO: needs more investigation
    target_link_libraries(ledger-core-coin-integration-tests ledger-core)
else()
    target_link_libraries(ledger-core-coin-integration-tests ledger-core-static)
endif()

target_link_libraries(ledger-core-coin-integration-tests ledger-test)
target_link_libraries(ledger-core-coin-integration-tests ledger-qt-host)
target_include_directories(ledger-core-coin-integration-tests PUBLIC ../../../core/src)
target_include_directories(ledger-core-coin-integration-tests PUBLIC ../../../qt-host)

include(CopyAndInstallImportedTargets)
if (SYS_OPENSSL)
    find_package(OpenSSL)
    copy_install_imported_targets(ledger-core-coin-integration-tests OpenSSL::Crypto)
else()
    copy_install_imported_targets(ledger-core-coin-integration-tests crypto)
endif()
add_test (NAME ledger-core-coin-integration COMMAND ledger-core-coin-integration-tests)
