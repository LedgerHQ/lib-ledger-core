cmake_minimum_required(VERSION 3.12)
project(ledger-core-ripple)

add_subdirectory(src)

if(BUILD_TESTS)
    message(STATUS "ledger-core-ripple tests are enabled")
    add_subdirectory(test)
else()
    message(STATUS "ledger-core-ripple tests are disabled")
endif()
