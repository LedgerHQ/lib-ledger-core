cmake_minimum_required(VERSION 3.12)
project(ledger-core-ethereum)

add_subdirectory(src)

if(BUILD_TESTS)
    message(STATUS "ledger-core-ethereum tests are enabled")
    add_subdirectory(test)
else()
    message(STATUS "ledger-core-etheurem tests are disabled")
endif()
