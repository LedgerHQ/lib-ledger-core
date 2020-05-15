project(ledger-core-ethereum-ethereum)

add_executable(ledger-core-ethereum-tests
    main.cpp
    Fixtures.cpp
    WalletTests.cpp
    KeychainTests.cpp
    SynchronizationTests.cpp
    TransactionTests.cpp
    AccountTests.cpp
    ExplorerTests.cpp
)

target_link_libraries(ledger-core-ethereum-tests ledger-core-ethereum-obj)
target_link_libraries(ledger-core-ethereum-tests ledger-core-bundle)
target_link_libraries(ledger-core-ethereum-tests ledger-core-integration-test)
target_link_libraries(ledger-core-ethereum-tests ledger-test)
target_link_libraries(ledger-core-ethereum-tests ledger-qt-host)
target_link_libraries(ledger-core-ethereum-tests gtest gtest_main)

add_test(NAME Ethereum-wallet COMMAND ledger-core-ethereum-tests --gtest_filter=EthereumWallets.*)
add_test(NAME Ethereum-transaction COMMAND ledger-core-ethereum-tests --gtest_filter=EthereumMakeTransaction.*)
add_test(NAME Ethereum-keychain COMMAND ledger-core-ethereum-tests --gtest_filter=EthereumKeychains.*)
add_test(NAME Ethereum-synchronization COMMAND ledger-core-ethereum-tests --gtest_filter=EthereumLikeWalletSynchronization.*)
add_test(NAME Ethereum-account COMMAND ledger-core-ethereum-tests --gtest_filter=EthereumAccounts.*)
add_test(NAME Ethereum-explorer COMMAND ledger-core-ethereum-tests --gtest_filter=EthereumExplorers.*)
