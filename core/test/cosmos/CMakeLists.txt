set( COSMOS_TESTS_NAME "ledger-core-cosmos-tests" )

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

file(GLOB ledger-core-fixtures-srcs
        "../fixtures/*.cpp"
        "../fixtures/*.h"
        )

add_executable(${COSMOS_TESTS_NAME}
  main.cpp
  Fixtures.cpp
  address_test.cpp
  db_test.cpp
  transactions_test.cpp
  AccountTests.cpp
  parsers_test.cpp
  ${ledger-core-fixtures-srcs}
  ../integration/BaseFixture.cpp
  ../integration/IntegrationEnvironment.cpp
  )
target_link_libraries(${COSMOS_TESTS_NAME} gtest gtest_main)
target_link_libraries(${COSMOS_TESTS_NAME} ledger-core-static)
target_link_libraries(${COSMOS_TESTS_NAME} ledger-test)
target_link_libraries(${COSMOS_TESTS_NAME} ledger-qt-host)
target_link_libraries(${COSMOS_TESTS_NAME} leveldb)
target_include_directories(${COSMOS_TESTS_NAME} PUBLIC ../../../qt-host)
target_include_directories(${COSMOS_TESTS_NAME} PUBLIC ../../../core/src)
target_include_directories(${COSMOS_TESTS_NAME} PUBLIC ../lib/libledger-test)
target_include_directories(${COSMOS_TESTS_NAME} PUBLIC ../../../core/lib/leveldb/include)

include(CopyAndInstallImportedTargets)
if (SYS_OPENSSL)
    find_package(OpenSSL)
    if(WIN32)
        target_link_libraries( ${COSMOS_TESTS_NAME} ws2_32 crypt32 )
    endif()	
    copy_install_imported_targets(${COSMOS_TESTS_NAME} OpenSSL::Crypto)
else()
    copy_install_imported_targets(${COSMOS_TESTS_NAME} crypto)
endif()
include(GoogleTest)
gtest_discover_tests(${COSMOS_TESTS_NAME})
