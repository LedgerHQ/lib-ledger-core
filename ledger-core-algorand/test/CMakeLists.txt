project(ledger-core-algorand-tests)

add_executable(ledger-core-algorand-tests
    main.cpp
    MsgpackHelpersTests.cpp
    AlgorandAccountTests.cpp
    AlgorandAddressTests.cpp
    AlgorandDatabaseTests.cpp
    AlgorandExplorerTests.cpp
    AlgorandSynchronizationTests.cpp
)

target_link_libraries(ledger-core-algorand-tests ledger-core-bundle)
target_link_libraries(ledger-core-algorand-tests ledger-test)
target_link_libraries(ledger-core-algorand-tests ledger-core-integration-test)
target_link_libraries(ledger-core-algorand-tests ledger-qt-host)
target_link_libraries(ledger-core-algorand-tests ledger-core-algorand-obj)
target_link_libraries(ledger-core-algorand-tests gtest gtest_main)

