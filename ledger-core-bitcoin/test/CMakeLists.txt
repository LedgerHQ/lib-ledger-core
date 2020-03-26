project(ledger-core-bitcoin-tests)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(NOT TARGET Core::ledger-core)
    find_package(ledger-core 0.1.0 REQUIRED)
endif()

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

add_executable(ledger-core-bitcoin-tests 
    main.cpp
    fixtures/Fixtures.cpp 
    fixtures/MediumXPubFixtures.cpp
    fixtures/BCHXPubFixtures.cpp
    fixtures/ZECXPubFixtures.cpp
    fixtures/TestnetXPubFixtures.cpp
    fixtures/BTGXPubFixtures.cpp
    fixtures/CoinSelectionXPubFixtures.cpp
    AddressTests.cpp
    ScriptTests.cpp
    KeychainTests.cpp
    WalletTests.cpp 
    AccountTests.cpp
    ExplorerTests.cpp
    CurrencyTests.cpp
    DatabaseTests.cpp
    TransactionTests.cpp
    SynchronizationTests.cpp
)

target_link_libraries(ledger-core-bitcoin-tests Core::ledger-core-static)
target_link_libraries(ledger-core-bitcoin-tests Core::ledger-test)
target_link_libraries(ledger-core-bitcoin-tests Core::ledger-core-integration-test)
target_link_libraries(ledger-core-bitcoin-tests Core::ledger-qt-host)
target_link_libraries(ledger-core-bitcoin-tests ledger-core-bitcoin-static)
target_link_libraries(ledger-core-bitcoin-tests gtest gtest_main)

set(gtest_force_shared_crt ON CACHE BOOL "Build gtest with shared runtime" FORCE)

add_subdirectory(lib/googletest)

add_test(NAME Bitcoin-script COMMAND ledger-core-bitcoin-tests --gtest_filter=BitcoinScript.*)
add_test(NAME Bitcoin-address COMMAND ledger-core-bitcoin-tests --gtest_filter=BitcoinAddress.*)
add_test(NAME Bitcoin-keychain COMMAND ledger-core-bitcoin-tests --gtest_filter=*BitcoinKeychains.*)
add_test(NAME Bitcoin-wallet COMMAND ledger-core-bitcoin-tests --gtest_filter=BitcoinWallets.*)
add_test(NAME Bitcoin-account COMMAND ledger-core-bitcoin-tests --gtest_filter=BitcoinAccounts.*)
add_test(NAME Bitcoin-explorer COMMAND ledger-core-bitcoin-tests --gtest_filter=BitcoinExplorers.*)
add_test(NAME Bitcoin-currency COMMAND ledger-core-bitcoin-tests --gtest_filter=BitcoinCurrencies.*)
add_test(NAME Bitcoin-database COMMAND ledger-core-bitcoin-tests --gtest_filter=BitcoinDatabases.*)
add_test(NAME Bitcoin-transaction COMMAND ledger-core-bitcoin-tests --gtest_filter=Bitcoin*Transactions.*)
add_test(NAME Bitcoin-synchronization COMMAND ledger-core-bitcoin-tests --gtest_filter=BitcoinSynchronization.*)
