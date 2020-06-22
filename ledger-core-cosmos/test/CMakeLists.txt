project(ledger-core-cosmos-tests)
include(GoogleTest)

add_executable(ledger-core-cosmos-tests
    main.cpp
    Fixtures.cpp
    AccountTests.cpp
    AddressTests.cpp
    SynchronizationTests.cpp
    DatabaseTests.cpp
    ParsersTests.cpp
    TransactionsTests.cpp
)

target_link_libraries(ledger-core-cosmos-tests ledger-core-bundle)
target_link_libraries(ledger-core-cosmos-tests ledger-test)
target_link_libraries(ledger-core-cosmos-tests ledger-core-integration-test)
target_link_libraries(ledger-core-cosmos-tests ledger-qt-host)
target_link_libraries(ledger-core-cosmos-tests ledger-core-cosmos-obj)
target_link_libraries(ledger-core-cosmos-tests gtest gtest_main)

gtest_discover_tests(ledger-core-cosmos-tests)
