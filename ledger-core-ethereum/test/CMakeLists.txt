project(ledger-core-ethereum-ethereum)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(ledger-core 0.1.0 REQUIRED)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

add_executable(ledger-core-ethereum-tests 
    main.cpp 
    Fixtures.cpp
    WalletTests.cpp
    KeychainTests.cpp 
    SynchronizationTests.cpp 
    TransactionTests.cpp
)

target_link_libraries(ledger-core-ethereum-tests Core::ledger-core-static)
target_link_libraries(ledger-core-ethereum-tests Core::ledger-test)
target_link_libraries(ledger-core-ethereum-tests Core::ledger-core-integration-test)
target_link_libraries(ledger-core-ethereum-tests Core::ledger-qt-host)
target_link_libraries(ledger-core-ethereum-tests ledger-core-ethereum-static)
target_link_libraries(ledger-core-ethereum-tests gtest gtest_main)

set(gtest_force_shared_crt ON CACHE BOOL "Build gtest with shared runtime" FORCE)

add_subdirectory(lib/googletest)

add_test(NAME Ethereum-wallet COMMAND ledger-core-ethereum-tests --gtest_filter=EthereumWallets.*)
add_test(NAME Ethereum-transaction COMMAND ledger-core-ethereum-tests --gtest_filter=EthereumMakeTransaction.*)
add_test(NAME Ethereum-keychain COMMAND ledger-core-ethereum-tests --gtest_filter=EthereumKeychains.*)
add_test(NAME Ethereum-synchronization COMMAND ledger-core-ethereum-tests --gtest_filter=EthereumLikeWalletSynchronization.*)
