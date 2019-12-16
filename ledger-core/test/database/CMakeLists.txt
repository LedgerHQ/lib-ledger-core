cmake_minimum_required(VERSION 3.0)
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

add_executable(ledger-core-database-tests
    main.cpp ServicesTests.cpp QueryFiltersTests.cpp BaseFixture.cpp BaseFixture.hpp
    IntegrationEnvironment.cpp IntegrationEnvironment.hpp DatabaseSociProxyTests.cpp
    MemoryDatabaseProxy.cpp MemoryDatabaseProxy.hpp SQLCipherTests.cpp
)

target_include_directories(
    ledger-core-database-tests PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> $<INSTALL_INTERFACE:include/ledger-test>
)

target_link_libraries(ledger-core-database-tests gtest gtest_main)
target_link_libraries(ledger-core-database-tests ledger-core-static)
target_link_libraries(ledger-core-database-tests ledger-test)
target_link_libraries(ledger-core-database-tests ledger-qt-host)

add_test(NAME ledger-core-database-tests COMMAND ledger-core-database-tests)
