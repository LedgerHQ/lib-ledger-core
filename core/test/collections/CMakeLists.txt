cmake_minimum_required(VERSION 3.0)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

add_executable(ledger-core-collections-tests main.cpp map_tests.cpp dynamics_tests.cpp)

target_link_libraries(ledger-core-collections-tests gtest gtest_main)
target_link_libraries(ledger-core-collections-tests ledger-core)

include(CopyAndInstallImportedTargets)
copy_install_imported_targets(ledger-core-collections-tests crypto ledger-core)

add_test (NAME ledger-core-collections-tests COMMAND ledger-core-collections-tests)