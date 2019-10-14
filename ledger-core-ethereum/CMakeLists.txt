cmake_minimum_required(VERSION 3.12)
project(ledger-core-ethereum)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
option(TARGET_JNI "Indicates wheter or not the toolchain must build for JNI or not" OFF)
option(BUILD_TESTS "Indicates wheter or not the toolchain must build the test or not" ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(UseBackportedModules)

# The project version number.
set(VERSION_MAJOR   0   CACHE STRING "Project major version number.")
set(VERSION_MINOR   1   CACHE STRING "Project minor version number.")
set(VERSION_PATCH   0   CACHE STRING "Project patch version number.")
mark_as_advanced(VERSION_MAJOR VERSION_MINOR VERSION_PATCH)

# C++ version and standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY build)

# Fix LLC-186: Add this flag to avoid crash for 10.10.x version
# https://stackoverflow.com/questions/41865537/how-does-apples-codesign-utility-decide-which-sha-algorithms-to-sign-a-shared
# Notes:
# > This is a "blind" fix, no available 10.10.x macOS machine,
# > Issue is specific to 10.10.x, 10.9.5 and > 10.10.x are fine
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9" CACHE STRING "Minimum OS X version to target for deployment: 10.9" FORCE)

set(CMAKE_MACOSX_RPATH 1)

add_definitions("-DSQLITE_HAS_CODEC")

string(FIND "${CMAKE_OSX_SYSROOT}" "iphone" IS_IOS)
if(IS_IOS GREATER_EQUAL 0 OR TARGET_JNI OR ANDROID)
    set(BUILD_TESTING OFF CACHE BOOL "iOS build fail otherwise" FORCE)
    set(BUILD_TESTS OFF CACHE BOOL "Cannot run tests for these options" FORCE)
endif()

enable_testing()

add_subdirectory(src)

string(FIND "${CMAKE_OSX_SYSROOT}" "iphone" IS_IOS)

if(IS_IOS LESS 0 AND BUILD_TESTS AND NOT IS_ANDROID)
    message(STATUS "Test are enabled")
    enable_testing()
    #add_subdirectory(test) TODO: uncomment to enable tests
else()
    message(STATUS "Test are disabled")
endif()
