cmake_minimum_required(VERSION 3.0)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

add_executable(ledger-core-utils-tests
    main.cpp EndianTests.cpp HexTests.cpp DerivationPathTest.cpp OptionTest.cpp EitherTest.cpp
    LazyTest.cpp DateParserTests.cpp DerivationSchemeTests.cpp ConfigurationMatchableTests.cpp
)

target_link_libraries(ledger-core-utils-tests gtest gtest_main)
target_link_libraries(ledger-core-utils-tests ledger-core-static)

target_include_directories(ledger-core-utils-tests PUBLIC ../../inc)

include(CopyAndInstallImportedTargets)
copy_install_imported_targets(ledger-core-utils-tests crypto)
add_test (NAME ledger-core-utils-tests COMMAND ledger-core-utils-tests)
