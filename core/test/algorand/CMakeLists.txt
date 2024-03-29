cmake_minimum_required(VERSION 3.0)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)

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
target_include_directories(ledger-core-algorand-tests PUBLIC ../../../core/src)
target_include_directories(ledger-core-algorand-tests PUBLIC ../lib/libledger-test)
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

if (SYS_LIBUV)
    copy_install_imported_targets(ledger-core-algorand-tests PkgConfig::UV)
else()
    copy_install_imported_targets(ledger-core-algorand-tests uv)
endif()

include(GoogleTest)
gtest_discover_tests(ledger-core-algorand-tests PROPERTIES "LABELS;unit")
