cmake_minimum_required(VERSION 3.0)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

add_executable(ledger-core-utils-tests
    main.cpp EndianTests.cpp HexTests.cpp DerivationPathTest.cpp OptionTest.cpp EitherTest.cpp
    LazyTest.cpp DateParserTests.cpp DerivationSchemeTests.cpp ConfigurationMatchableTests.cpp
)

target_link_libraries(ledger-core-utils-tests gtest gtest_main)
target_link_libraries(ledger-core-utils-tests ledger-core-bundle)

add_test (NAME ledger-core-utils-tests COMMAND ledger-core-utils-tests)
