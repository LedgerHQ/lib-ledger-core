cmake_minimum_required(VERSION 3.15)
project(ledger-core)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
option(TARGET_JNI "Indicates wheter or not the toolchain must build for JNI or not" OFF)
option(BUILD_TESTS "Indicates wheter or not the toolchain must build the test or not" ON)
option(TESTS_UPDATE_HTTP_CACHE "Update HTTP cache for tests" OFF)
option(TESTS_USE_EXTERNAL_WEB_SERVICES "Use external web-services instead of cached HTTP responses" OFF)
option(NIX_BUILD "Indicates whether this build is using nix dependencies." OFF)
option(SYS_LIBUV "Indicates whether this build is using system libuv." OFF)
option(SYS_SECP256K1 "Indicates whether this build is using system libsecp256k1." OFF)
option(SYS_OPENSSL "Indicates whether this build is using system openssl." OFF)
if(NIX_BUILD)
    set(SYS_LIBUV ON CACHE BOOL "Building with Nix implies using system UV" FORCE)
    set(SYS_SECP256K1 ON CACHE BOOL "Building with Nix implies using system secp256k1" FORCE)
    set(SYS_OPENSSL ON CACHE BOOL "Building with Nix implies using system openssl" FORCE)
endif()

# Update Cmake Module Path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake-modules")
include(UseBackportedModules)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake-scripts")

# General internal functions
function (unsetListOfVarsStartingWith _prefix)
    get_cmake_property(_vars VARIABLES)
    string (REGEX MATCHALL "(^|;)${_prefix}[A-Za-z0-9_]*" _matchedVars "${_vars}")
	FOREACH(var ${_matchedVars})
		unset (${var} CACHE)
	ENDFOREACH()
endfunction()

# Include cmake scripts
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/CCache.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Coverage.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Cotire.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake-scripts/c++-standards.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake-scripts/tools.cmake")

cxx_14()
clang_tidy("-checks=-*,clang-analyzer-*,-clang-analyzer-cplusplus*,cppcoreguidelines-*,modernize-*")            # add cmake conf option CLANG_TIDY=ON to activate
include_what_you_use()  # add cmake conf option IWYU=ON to activate

# The project version number.
set(VERSION_MAJOR   4    CACHE STRING "Project major version number.")
set(VERSION_MINOR   3    CACHE STRING "Project minor version number.")
set(VERSION_PATCH   1    CACHE STRING "Project patch version number.")
mark_as_advanced(VERSION_MAJOR VERSION_MINOR VERSION_PATCH)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY build)
list(APPEND INCLUDE_DIRECTORIES core/test/include/)

# Setup BOOST
set(BOOST_INCLUDEDIR "${CMAKE_CURRENT_SOURCE_DIR}/core/lib/boost/")

set(CMAKE_MACOSX_RPATH 1)

add_definitions("-DSQLITE_HAS_CODEC")

# To add SSL support
if (SSL_SUPPORT)
    add_definitions("-DSSL_SUPPORT")
endif()

string(FIND "${CMAKE_OSX_SYSROOT}" "iphone" IS_IOS)
if(IS_IOS GREATER_EQUAL 0 OR TARGET_JNI OR ANDROID)
    set(BUILD_TESTING OFF CACHE BOOL "iOS build fail otherwise" FORCE)
    set(BUILD_TESTS OFF CACHE BOOL "Cannot run tests for these options" FORCE)
endif()

enable_testing()

add_subdirectory(doc)
add_subdirectory(core)

# Fix LLC-186: Add this flag to avoid crash for 10.10.x version
# https://stackoverflow.com/questions/41865537/how-does-apples-codesign-utility-decide-which-sha-algorithms-to-sign-a-shared
# Notes:
# > This is a "blind" fix, no available 10.10.x macOS machine,
# > Issue is specific to 10.10.x, 10.9.5 and > 10.10.x are fine
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X version to target for deployment: 10.9" FORCE)

