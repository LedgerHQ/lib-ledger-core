cmake_minimum_required(VERSION 3.0)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)
include_directories(${CMAKE_BINARY_DIR}/include/ledger/core)


add_executable(ledger-core-collections-tests main.cpp map_tests.cpp dynamics_tests.cpp)

target_link_libraries(ledger-core-collections-tests gtest gtest_main)
target_link_libraries(ledger-core-collections-tests ledger-core-static)

include(CopyAndInstallImportedTargets)
if (SYS_OPENSSL)
    find_package(OpenSSL)
    if(WIN32)
        target_link_libraries( ledger-core-collections-tests ws2_32 crypt32 )
    endif()
    copy_install_imported_targets(ledger-core-collections-tests OpenSSL::Crypto)
else()
    copy_install_imported_targets(ledger-core-collections-tests crypto)
endif()

include(GoogleTest)
gtest_discover_tests(ledger-core-collections-tests PROPERTIES "LABELS;unit")
