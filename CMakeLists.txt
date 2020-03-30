cmake_minimum_required(VERSION 3.10)
project(lib-ledger-core)

set(BUILD_COINS "" CACHE STRING "List all the coins to build along side core library")
mark_as_advanced(BUILD_COINS)

add_subdirectory(ledger-core)

# allows to provide coin list separated by commma or whitespace
string(REPLACE " " ";" BUILD_COINS "${BUILD_COINS}")
string(REPLACE "," ";" BUILD_COINS "${BUILD_COINS}")

foreach(COIN ${BUILD_COINS})
    add_subdirectory(ledger-core-${COIN})
endforeach()