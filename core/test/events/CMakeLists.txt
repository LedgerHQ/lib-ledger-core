cmake_minimum_required(VERSION 3.0)
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)

include_directories(../lib/libledger-test/)

add_executable(ledger-core-events-tests main.cpp events_test.cpp)

target_link_libraries(ledger-core-events-tests gtest gtest_main)
target_link_libraries(ledger-core-events-tests ledger-core-static)
target_link_libraries(ledger-core-events-tests ledger-test)
include(CopyAndInstallImportedTargets)
if (SYS_OPENSSL)
    find_package(OpenSSL)
    if(WIN32)
        target_link_libraries( ledger-core-events-tests ws2_32 crypt32 )
    endif()
    copy_install_imported_targets(ledger-core-events-tests OpenSSL::Crypto)
else()
    copy_install_imported_targets(ledger-core-events-tests crypto)
endif()

if (SYS_LIBUV)
    copy_install_imported_targets(ledger-core-events-tests PkgConfig::UV)
else()
    copy_install_imported_targets(ledger-core-events-tests uv)
endif()

include(GoogleTest)
gtest_discover_tests(ledger-core-events-tests PROPERTIES "LABELS;unit")
