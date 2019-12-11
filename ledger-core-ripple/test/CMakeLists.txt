project(ledger-core-ripple-tests)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(ledger-core 0.1.0 REQUIRED)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

add_executable(
    ledger-core-ripple-tests main.cpp
    Common.cpp AddressTest.cpp Fixtures.cpp RippleKeychainTest.cpp TransactionTest.cpp
    SynchronizationTest.cpp
)

target_link_libraries(ledger-core-ripple-tests Core::ledger-core-static)
target_link_libraries(ledger-core-ripple-tests Core::ledger-test)
target_link_libraries(ledger-core-ripple-tests Core::ledger-core-integration-test)
target_link_libraries(ledger-core-ripple-tests Core::ledger-qt-host)
target_link_libraries(ledger-core-ripple-tests ledger-core-ripple-static)
target_link_libraries(ledger-core-ripple-tests gtest gtest_main)

set(gtest_force_shared_crt ON CACHE BOOL "Build gtest with shared runtime" FORCE)

add_subdirectory(lib/googletest)

add_test (NAME Ripple-address COMMAND ledger-core-ripple-tests --gtest_filter=RippleAddress.*)
add_test (NAME Ripple-keychain COMMAND ledger-core-ripple-tests --gtest_filter=RippleKeychains.*)
add_test (NAME Ripple-transaction COMMAND ledger-core-ripple-tests --gtest_filter=RippleMakeTransaction.*)
add_test (NAME Ripple-sync COMMAND ledger-core-ripple-tests --gtest_filter=RippleLikeWalletSynchronization.*)
