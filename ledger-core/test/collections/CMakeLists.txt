cmake_minimum_required(VERSION 3.0)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)
include_directories(${CMAKE_BINARY_DIR}/include/ledger/core)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

add_executable(ledger-core-collections-tests main.cpp MapTests.cpp DynamicsTests.cpp)

target_link_libraries(ledger-core-collections-tests gtest gtest_main)
target_link_libraries(ledger-core-collections-tests ledger-core-static)

include(CopyAndInstallImportedTargets)
copy_install_imported_targets(ledger-core-collections-tests crypto)

add_test (NAME ledger-core-collections-tests COMMAND ledger-core-collections-tests)
