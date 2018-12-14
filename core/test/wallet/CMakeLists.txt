cmake_minimum_required(VERSION 3.0)
include_directories(.)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${gmock_SOURCE_DIR}/include ${gmock_SOURCE_DIR})
include_directories(../../src/)
if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

add_executable(ledger-core-wallet-tests 
main.cpp 
InMemoryPartialBlocksDB_test.cpp 
BlocksSynchronizer_test.cpp
Helpers.cpp
ChangeKeychain_test.cpp
PersistentBlockchainDatabase_test.cpp
AccountSynchronizer_test.cpp
InMemoryBlockchainDatabase_test.cpp
)

target_link_libraries(ledger-core-wallet-tests gtest gtest_main)
target_link_libraries(ledger-core-wallet-tests gmock)
target_link_libraries(ledger-core-wallet-tests ledger-core)

include(CopyAndInstallImportedTargets)
copy_install_imported_targets(ledger-core-wallet-tests crypto ledger-core)

add_test (NAME ledger-core-wallet-tests COMMAND ledger-core-wallet-tests)