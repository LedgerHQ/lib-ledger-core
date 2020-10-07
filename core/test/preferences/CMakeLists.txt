cmake_minimum_required(VERSION 3.0)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

include_directories(../lib/libledger-test/)

add_executable(ledger-core-preferences-tests main.cpp preferences_test.cpp)

target_link_libraries(ledger-core-preferences-tests gtest gtest_main)
target_link_libraries(ledger-core-preferences-tests ledger-core-static)
target_link_libraries(ledger-core-preferences-tests ledger-test)

include(CopyAndInstallImportedTargets)
if (SYS_OPENSSL)
    find_package(OpenSSL)
    if(WIN32)
        target_link_libraries( ledger-core-preferences-tests crypt32 )
    endif()
    copy_install_imported_targets(ledger-core-preferences-tests OpenSSL::Crypto)
else()
    copy_install_imported_targets(ledger-core-preferences-tests crypto)
endif()
add_test (NAME ledger-core-preferences-tests COMMAND ledger-core-preferences-tests)
