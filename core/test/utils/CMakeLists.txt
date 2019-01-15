cmake_minimum_required(VERSION 3.0)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)
if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

add_executable(ledger-core-utils-tests main.cpp endian_tests.cpp hex_tests.cpp derivation_path_test.cpp option_test.cpp either_test.cpp lazy_test.cpp date_parser_tests.cpp derivation_scheme_tests.cpp configuration_matchable_tests.cpp)

target_link_libraries(ledger-core-utils-tests gtest gtest_main)
target_link_libraries(ledger-core-utils-tests ledger-core-static)

target_include_directories(ledger-core-utils-tests PUBLIC ../../../core/src/)

include(CopyAndInstallImportedTargets)
copy_install_imported_targets(ledger-core-utils-tests crypto)
add_test (NAME ledger-core-utils-tests COMMAND ledger-core-utils-tests)