cmake_minimum_required(VERSION 3.0)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

add_executable(ledger-core-algorand-tests
    main.cpp
    MsgpackHelpersTests.cpp
    AlgorandAccountTests.cpp
    AlgorandAddressTests.cpp
    AlgorandExplorerTests.cpp
    AlgorandDatabaseTests.cpp
    AlgorandSynchronizationTests.cpp
    AlgorandSerializationTests.cpp
  ../integration/BaseFixture.cpp
  ../integration/IntegrationEnvironment.cpp
)

target_link_libraries(ledger-core-algorand-tests gtest gtest_main)
target_link_libraries(ledger-core-algorand-tests ledger-core-static)
target_link_libraries(ledger-core-algorand-tests ledger-test)
target_link_libraries(ledger-core-algorand-tests leveldb)
target_link_libraries(ledger-core-algorand-tests ledger-qt-host)
target_include_directories(ledger-core-algorand-tests PUBLIC ../../../qt-host)
target_include_directories(ledger-core-algorand-tests PUBLIC ../../../core/src)
target_include_directories(ledger-core-algorand-tests PUBLIC ../lib/libledger-test)
target_include_directories(ledger-core-algorand-tests PUBLIC ../../../core/lib/leveldb/include)
include_directories(../lib/libledger-test/)

target_include_directories(ledger-core-algorand-tests PRIVATE ${CMAKE_SOURCE_DIR}/src/thirdparty/msgpack-c/include)

include(CopyAndInstallImportedTargets)
if (SYS_OPENSSL)
    find_package(OpenSSL)
    if(WIN32)
        target_link_libraries( ledger-core-algorand-tests ws2_32 crypt32 )
    endif()
    copy_install_imported_targets(ledger-core-algorand-tests OpenSSL::Crypto ledger-core)
else()
    copy_install_imported_targets(ledger-core-algorand-tests crypto ledger-core)
endif()
add_test (NAME ledger-core-algorand-tests COMMAND ledger-core-algorand-tests)

