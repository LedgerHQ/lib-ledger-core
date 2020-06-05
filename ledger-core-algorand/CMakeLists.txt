cmake_minimum_required(VERSION 3.12)
project(ledger-core-algorand)

add_subdirectory(src)

if(IS_IOS LESS 0 AND BUILD_TESTS AND NOT IS_ANDROID)
    message(STATUS "ledger-core-algorand tests are enabled")
    enable_testing()
    add_subdirectory(test)
else()
    message(STATUS "ledger-core-algorand tests are disabled")
endif()

