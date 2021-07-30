set( TEZOS_TESTS_NAME "ledger-core-tezos-tests" )

cmake_minimum_required(VERSION 3.0)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

add_executable(${TEZOS_TESTS_NAME}
    main.cpp
    Fixtures.cpp
    address_test.cpp
    transaction_test.cpp
    ED25519_transaction_test.cpp
    SECP256K1_transaction_test.cpp
    P256_transaction_test.cpp
    ../integration/BaseFixture.cpp
    ../integration/IntegrationEnvironment.cpp
)

target_link_libraries(${TEZOS_TESTS_NAME} gtest gtest_main)
target_link_libraries(${TEZOS_TESTS_NAME} ledger-core-static)
target_link_libraries(${TEZOS_TESTS_NAME} ledger-test)
target_link_libraries(${TEZOS_TESTS_NAME} leveldb)
target_include_directories(${TEZOS_TESTS_NAME} PUBLIC ../../../qt-host)
target_include_directories(${TEZOS_TESTS_NAME} PUBLIC ../../../core/src)
target_include_directories(${TEZOS_TESTS_NAME} PUBLIC ../lib/libledger-test)
target_include_directories(${TEZOS_TESTS_NAME} PUBLIC ../../../core/lib/leveldb/include)
include_directories(../lib/libledger-test/)

include(CopyAndInstallImportedTargets)
if (SYS_OPENSSL)
    find_package(OpenSSL)
    if(WIN32)
        target_link_libraries( ledger-core-tezos-tests crypt32 ws2_32)
    endif()
    copy_install_imported_targets(ledger-core-tezos-tests OpenSSL::Crypto ledger-core)
else()
    copy_install_imported_targets(ledger-core-tezos-tests crypto ledger-core)
endif()
add_test (NAME ${TEZOS_TESTS_NAME} COMMAND ${TEZOS_TESTS_NAME})
