cmake_minimum_required(VERSION 3.7)
project(ledger-core)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
option(TARGET_JNI "Indicates wheter or not the toolchain must build for JNI or not" OFF)
option(BUILD_TESTS "Indicates wheter or not the toolchain must build the test or not" ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(UseBackportedModules)

# The project version number.
set(VERSION_MAJOR   0   CACHE STRING "Project major version number.")
set(VERSION_MINOR   0   CACHE STRING "Project minor version number.")
set(VERSION_PATCH   1   CACHE STRING "Project patch version number.")
mark_as_advanced(VERSION_MAJOR VERSION_MINOR VERSION_PATCH)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY build)
list(APPEND INCLUDE_DIRECTORIES core/test/include/)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_MACOSX_RPATH 1)

string(FIND "${CMAKE_OSX_SYSROOT}" "iphone" IS_IOS)
if(IS_IOS GREATER_EQUAL 0 OR TARGET_JNI)
    set(BUILD_TESTING OFF CACHE BOOL "iOS build fail otherwise" FORCE)
    set(ENABLE_TESTS OFF CACHE BOOL "Cannot run tests for these options" FORCE)
endif()

add_subdirectory(doc)
add_subdirectory(core)

if (ENABLE_TESTS)
    add_subdirectory(qt-host)
endif()

enable_testing()
