cmake_minimum_required(VERSION 3.0)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

add_executable(ledger-core-manager
    main.cpp
  ../integration/BaseFixture.cpp
  ../integration/IntegrationEnvironment.cpp
)

target_link_libraries(ledger-core-manager gtest gtest_main)
target_link_libraries(ledger-core-manager ledger-core-static)
target_link_libraries(ledger-core-manager ledger-test)
target_link_libraries(ledger-core-manager leveldb)
target_link_libraries(ledger-core-manager ledger-qt-host)
target_include_directories(ledger-core-manager PUBLIC ../../../qt-host)
target_include_directories(ledger-core-manager PUBLIC ../../../core/src)
target_include_directories(ledger-core-manager PUBLIC ../lib/libledger-test)
target_include_directories(ledger-core-manager PUBLIC ../../../core/lib/leveldb/include)
include_directories(../lib/libledger-test/)

target_include_directories(ledger-core-manager PRIVATE ${CMAKE_SOURCE_DIR}/src/thirdparty/msgpack-c/include)

include(CopyAndInstallImportedTargets)
copy_install_imported_targets(ledger-core-manager crypto ledger-core)


