project(ledger-core-bitcoin-tests)

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

target_link_libraries(ledger-core-bitcoin-tests ledger-core-bundle)
target_link_libraries(ledger-core-bitcoin-tests ledger-test)
target_link_libraries(ledger-core-bitcoin-tests ledger-core-integration-test)
target_link_libraries(ledger-core-bitcoin-tests ledger-qt-host)
target_link_libraries(ledger-core-bitcoin-tests ledger-core-bitcoin-obj)
target_link_libraries(ledger-core-bitcoin-tests gtest gtest_main)

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
