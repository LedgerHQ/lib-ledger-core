project(ledger-core-ripple-tests)

add_executable(
    ledger-core-ripple-tests main.cpp
    AddressTest.cpp Fixtures.cpp RippleKeychainTest.cpp TransactionTest.cpp
    SynchronizationTest.cpp AccountTests.cpp
)

target_link_libraries(ledger-core-ripple-tests ledger-core-ripple-obj)
target_link_libraries(ledger-core-ripple-tests ledger-core-bundle)
target_link_libraries(ledger-core-ripple-tests ledger-core-integration-test)
target_link_libraries(ledger-core-ripple-tests ledger-test)
target_link_libraries(ledger-core-ripple-tests ledger-qt-host)
target_link_libraries(ledger-core-ripple-tests gtest gtest_main)

add_test (NAME Ripple-address COMMAND ledger-core-ripple-tests --gtest_filter=RippleAddress.*)
add_test (NAME Ripple-keychain COMMAND ledger-core-ripple-tests --gtest_filter=RippleKeychains.*)
add_test (NAME Ripple-transaction COMMAND ledger-core-ripple-tests --gtest_filter=RippleMakeTransaction.*)
add_test (NAME Ripple-sync COMMAND ledger-core-ripple-tests --gtest_filter=RippleLikeWalletSynchronization.*)
add_test (NAME Ripple-account COMMAND ledger-core-ripple-tests --gtest_filter=RippleAccounts.*)
