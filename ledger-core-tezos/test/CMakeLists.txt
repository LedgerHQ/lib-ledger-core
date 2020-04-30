project(ledger-core-tezos-tests)

add_executable(
    ledger-core-tezos-tests main.cpp
    AddressTest.cpp Fixtures.cpp TransactionTests.cpp SynchronizationTests.cpp
    AccountTests.cpp
)

target_link_libraries(ledger-core-tezos-tests Core::ledger-core-obj)
target_link_libraries(ledger-core-tezos-tests Core::ledger-test)
target_link_libraries(ledger-core-tezos-tests Core::ledger-core-integration-test)
target_link_libraries(ledger-core-tezos-tests Core::ledger-qt-host)
target_link_libraries(ledger-core-tezos-tests ledger-core-tezos-obj)
target_link_libraries(ledger-core-tezos-tests gtest gtest_main)

add_test (NAME Tezos-address COMMAND ledger-core-tezos-tests --gtest_filter=TezosAddress.*)
add_test (NAME Tezos-transaction COMMAND ledger-core-tezos-tests --gtest_filter=TezosMakeTransaction.*)
add_test (NAME Tezos-sync COMMAND ledger-core-tezos-tests --gtest_filter=TezosLikeWalletSynchronization.*)
add_test (NAME Tezos-account COMMAND ledger-core-tezos-tests --gtest_filter=TezosAccounts.*)
