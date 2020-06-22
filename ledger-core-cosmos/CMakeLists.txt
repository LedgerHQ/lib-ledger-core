cmake_minimum_required(VERSION 3.12)
project(ledger-core-cosmos)

add_subdirectory(src)

if(IS_IOS LESS 0 AND BUILD_TESTS AND NOT IS_ANDROID)
    message(STATUS "ledger-core-cosmos tests are enabled")
    add_subdirectory(test)
else()
    message(STATUS "ledger-core-cosmos tests are disabled")
endif()
