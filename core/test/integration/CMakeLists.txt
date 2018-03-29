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
add_executable(ledger-core-integration-tests
        main.cpp
        initialization_test.cpp
        p2pkh_keychain_test.cpp
        ledger_api_blockchain_explorer_tests.cpp
        wallet_database_tests.cpp
        synchronization_tests.cpp
        account_info_test.cpp
        wallet_tests.cpp
        pool_tests.cpp
        BaseFixture.cpp BaseFixture.h accounts_public_interfaces_test.cpp
        account_creation_tests.cpp
        ${ledger-core-fixtures-srcs} account_blockchain_observation.cpp IntegrationEnvironment.cpp IntegrationEnvironment.h bitcoin_transaction_tests.cpp)

target_link_libraries(ledger-core-integration-tests gtest gtest_main)
target_link_libraries(ledger-core-integration-tests ledger-core)
target_link_libraries(ledger-core-integration-tests ledger-test)
target_link_libraries(ledger-core-integration-tests ledger-qt-host)
target_include_directories(ledger-core-integration-tests PUBLIC ../../../core/src)
target_include_directories(ledger-core-integration-tests PUBLIC ../../../qt-host)
add_test (NAME ledger-core-integration-tests COMMAND ledger-core-integration-tests)