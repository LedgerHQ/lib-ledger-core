project(ledger-core-tezos-tests)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(ledger-core 0.1.0 REQUIRED)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

add_executable(
    ledger-core-tezos-tests main.cpp
    AddressTest.cpp Fixtures.cpp TransactionTests.cpp SynchronizationTests.cpp
)

target_link_libraries(ledger-core-tezos-tests Core::ledger-core-static)
target_link_libraries(ledger-core-tezos-tests Core::ledger-test)
target_link_libraries(ledger-core-tezos-tests Core::ledger-core-integration-test)
target_link_libraries(ledger-core-tezos-tests Core::ledger-qt-host)
target_link_libraries(ledger-core-tezos-tests ledger-core-tezos-static)
target_link_libraries(ledger-core-tezos-tests gtest gtest_main)

set(gtest_force_shared_crt ON CACHE BOOL "Build gtest with shared runtime" FORCE)

add_subdirectory(lib/googletest)

add_test (NAME Tezos-address COMMAND ledger-core-tezos-tests --gtest_filter=TezosAddress.*)
add_test (NAME Tezos-transaction COMMAND ledger-core-tezos-tests --gtest_filter=TezosMakeTransaction.*)
add_test (NAME Tezos-sync COMMAND ledger-core-tezos-tests --gtest_filter=TezosLikeWalletSynchronization.*)
