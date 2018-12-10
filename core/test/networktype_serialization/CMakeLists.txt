cmake_minimum_required(VERSION 3.0)
include_directories(.)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${gmock_SOURCE_DIR}/include ${gmock_SOURCE_DIR})
include_directories(../../src/)
include_directories(../../lib/)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)


add_compile_definitions(LIBCORE_EXPORT=)

add_executable(ledger-networktype-serialization-test
main.cpp 
bitcoin_test.cpp
../../src/math/BigInt.cpp
../../src/collections/strings.cpp
../../src/utils/endian.cpp
)

target_link_libraries(ledger-networktype-serialization-test gtest gtest_main)
target_link_libraries(ledger-networktype-serialization-test gmock)
target_link_libraries(ledger-networktype-serialization-test bigd)

include(CopyAndInstallImportedTargets)
copy_install_imported_targets(ledger-bitcoin-serialize-test)

add_test (NAME ledger-bitcoin-serialize-test COMMAND ledger-bitcoin-serialize-test)

target_include_directories(ledger-networktype-serialization-test PUBLIC .)
target_include_directories(ledger-networktype-serialization-test PUBLIC ../../lib/bigd)
target_include_directories(ledger-networktype-serialization-test PUBLIC ../../lib/fmt-3.0.0)
target_include_directories(ledger-networktype-serialization-test PUBLIC ../../lib/rapidjson/include)
target_include_directories(ledger-networktype-serialization-test PUBLIC ../../test/lib/boost)
target_include_directories(ledger-networktype-serialization-test PUBLIC ../../lib/cereal/)