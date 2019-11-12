project(ledger-core-ripple-tests)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(../inc)
include_directories(../../ledger-core/inc)
include_directories(../lib/leveldb/include)
include_directories(../lib/fmt/include)
include_directories(../lib/bigd)
include_directories(../lib/secp256k1)

set(gtest_force_shared_crt ON CACHE BOOL "Build gtest with shared runtime" FORCE)

link_directories(../../ledger-core/build/src)
link_directories(../../ledger-core/build/lib/leveldb)
link_directories(../../ledger-core/build/lib/openssl/crypto)
link_directories(../../ledger-core/build/lib/openssl/ssl)
link_directories(../../ledger-core/lib/secp256k1/lib)

add_subdirectory(lib/googletest)
add_subdirectory(lib/libledger-test)
add_subdirectory(address)
