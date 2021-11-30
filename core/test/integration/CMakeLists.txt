cmake_minimum_required(VERSION 3.0)
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)

include_directories(../lib/libledger-test/)

file(GLOB ledger-core-fixtures-srcs
        "../fixtures/*.cpp"
        "../fixtures/*.h"
        "../cosmos/Fixtures.cpp"
        "../cosmos/Fixtures.hpp"
        )

file(GLOB_RECURSE TEST_FILES *.cpp)
add_executable(ledger-core-integration-tests
        ${ledger-core-fixtures-srcs}
        ${TEST_FILES}
        IntegrationEnvironment.h
        )
if (SYS_OPENSSL)
    find_package(OpenSSL)
    if(WIN32)
        target_link_libraries( ledger-core-integration-tests ws2_32 crypt32 )
    endif()
    target_link_libraries(ledger-core-integration-tests OpenSSL::Crypto)
else()
    target_link_libraries(ledger-core-integration-tests crypto)
endif()

target_link_libraries(ledger-core-integration-tests gtest gtest_main)
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
target_include_directories(ledger-core-integration-tests PUBLIC ../../../core/src)

include(CopyAndInstallImportedTargets)
if (SYS_OPENSSL)
    find_package(OpenSSL)
    copy_install_imported_targets(ledger-core-integration-tests OpenSSL::Crypto)
else()
    copy_install_imported_targets(ledger-core-integration-tests crypto)
endif()

if (SYS_LIBUV)
    copy_install_imported_targets(ledger-core-integration-tests PkgConfig::UV)
else()
    copy_install_imported_targets(ledger-core-integration-tests uv)
endif()

include(GoogleTest)
gtest_discover_tests(ledger-core-integration-tests PROPERTIES "LABELS;integration")