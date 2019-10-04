cmake_minimum_required(VERSION 3.0)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/core/src)
include_directories(${CMAKE_SOURCE_DIR}/core/lib/bigd)
include_directories(${CMAKE_SOURCE_DIR}/core/lib/fmt/include)
include_directories(${CMAKE_SOURCE_DIR}/core/lib/boost)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

add_executable(ledger-core-bigint-tests 
    main.cpp
    bigint_tests.cpp
    ${CMAKE_SOURCE_DIR}/core/src/math/BigInt.cpp
    ${CMAKE_SOURCE_DIR}/core/src/utils/endian.cpp
    ${CMAKE_SOURCE_DIR}/core/src/collections/strings.cpp)

target_compile_definitions(ledger-core-bigint-tests PUBLIC LEDGER_CORE_STATIC_DEFINE)
target_link_libraries(ledger-core-bigint-tests gtest gmock gtest_main)
target_link_libraries(ledger-core-bigint-tests bigd)
add_test (NAME ledger-core-bigint-tests COMMAND ledger-core-bigint-tests)
