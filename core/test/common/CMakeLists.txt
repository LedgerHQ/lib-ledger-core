cmake_minimum_required(VERSION 3.0)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)

add_executable(ledger-core-common-tests 
    main.cpp
    balance_history.cpp
    operation.cpp 
)

target_link_libraries(ledger-core-common-tests gtest gtest_main)
target_link_libraries(ledger-core-common-tests ledger-core-static)
target_link_libraries(ledger-core-common-tests ledger-test)
include_directories(../lib/libledger-test/)

include(CopyAndInstallImportedTargets)
if (SYS_OPENSSL)
    find_package(OpenSSL)
    if(WIN32)
        target_link_libraries( ledger-core-common-tests ws2_32 crypt32 )
    endif()
    copy_install_imported_targets(ledger-core-common-tests OpenSSL::Crypto)
else()
    copy_install_imported_targets(ledger-core-common-tests crypto)
endif()

include(GoogleTest)
gtest_discover_tests(ledger-core-common-tests PROPERTIES "LABELS;unit")

