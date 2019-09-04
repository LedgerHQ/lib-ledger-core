cmake_minimum_required(VERSION 3.12)
project(ledger-core-ripple)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
option(TARGET_JNI "Indicates whether or not the toolchain must build for JNI or not" OFF)
option(BUILD_TESTS "Indicates whether or not the toolchain must build the test or not" ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(UseBackportedModules)

include(ProjectSettings)
set_project_settings(0.1.0)

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

include(AutoFormat)

file(GLOB_RECURSE CMAKE_FILES "CMakeLists.txt")
list(FILTER CMAKE_FILES EXCLUDE REGEX "${CMAKE_CURRENT_SOURCE_DIR}/(test/)?lib/*")
set_cmake_format(${CMAKE_CURRENT_SOURCE_DIR}/../.cmake-format.yaml "${CMAKE_FILES}")

file(GLOB_RECURSE CXX_FILES "*.hpp" "*.cpp")
list(FILTER CXX_FILES EXCLUDE REGEX "${CMAKE_CURRENT_SOURCE_DIR}/(test/)?(lib|build|inc/core/api|inc/core/jni)/*")
set_clang_format("${CXX_FILES}"))

add_subdirectory(src)

string(FIND "${CMAKE_OSX_SYSROOT}" "iphone" IS_IOS)

if(BUILD_TESTS)
    message(STATUS "Tests are enabled")
    enable_testing()
    add_subdirectory(test)
else()
    message(STATUS "Tests are disabled")
endif()
