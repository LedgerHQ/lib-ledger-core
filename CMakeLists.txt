cmake_minimum_required(VERSION 3.10)
project(lib-ledger-core)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(UseBackportedModules)

include(ProjectSettings)
set_project_settings(0.1.0)

option(TARGET_JNI "Indicates whether the toolchain must build for JNI or not" OFF)
option(BUILD_TESTS "Indicates whether the toolchain must build the test or not" ON)

set(BUILD_COINS "" CACHE STRING "List all the coins to build along side core library")
mark_as_advanced(BUILD_COINS)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Fix LLC-186: Add this flag to avoid crash for 10.10.x version
# https://stackoverflow.com/questions/41865537/how-does-apples-codesign-utility-decide-which-sha-algorithms-to-sign-a-shared
# Notes:
# > This is a "blind" fix, no available 10.10.x macOS machine,
# > Issue is specific to 10.10.x, 10.9.5 and > 10.10.x are fine
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9" CACHE STRING "Minimum OS X version to target for deployment: 10.9" FORCE)
set(CMAKE_MACOSX_RPATH 1)

# Various definitions used to conditionally compile some part of our code or dependencies
add_definitions("-DSQLITE_HAS_CODEC")

if(PG_SUPPORT)
    add_definitions("-DPG_SUPPORT")
endif()

if(SSL_SUPPORT)
    add_definitions("-DSSL_SUPPORT")
endif()

if(TARGET_JNI)
  add_definitions(-DTARGET_JNI=1)

  if(NOT ANDROID)
    find_package(JNI REQUIRED)
  endif()
endif()

string(FIND "${CMAKE_OSX_SYSROOT}" "iphone" IS_IOS)

if(IS_IOS GREATER_EQUAL 0 OR TARGET_JNI OR ANDROID)
    set(BUILD_TESTING OFF CACHE BOOL "iOS build fail otherwise" FORCE)
    set(BUILD_TESTS OFF CACHE BOOL "Cannot run tests for these options" FORCE)
endif()

if(BUILD_TESTS AND APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif()

add_subdirectory(ledger-core)

add_library(ledger-core-bundle SHARED)
target_link_libraries(ledger-core-bundle PUBLIC ledger-core-obj)

# TODO: this line is not mandatory on Ubuntu 19.04 - we have to confirm if this is mandatory in other platforms to work properly
# if(UNIX AND NOT APPLE AND NOT ANDROID)
#   target_link_libraries(ledger-core-bundle PUBLIC -static-libstdc++)
# endif()

if (TARGET_JNI)
    target_include_directories(ledger-core-bundle PRIVATE ${JNI_INCLUDE_DIRS})
    target_link_libraries(ledger-core-bundle PRIVATE ${JNI_LIBRARIES})
endif ()

# allows to provide coin list separated by commma or whitespace
string(REPLACE " " ";" BUILD_COINS "${BUILD_COINS}")
string(REPLACE "," ";" BUILD_COINS "${BUILD_COINS}")

foreach(COIN ${BUILD_COINS})
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ledger-core-${COIN}/CMakeLists.txt")
    add_subdirectory(ledger-core-${COIN})
    target_link_libraries(ledger-core-bundle PUBLIC ledger-core-${COIN}-obj)
  else()
    message(WARNING "coin '${COIN}' not found")
  endif()
endforeach()

# Add all the API objects (some needs to be compiled, and because they are shared, we need to
# compile them only once).
file(GLOB API_SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/bundle/api/*")
add_library(ledger-core-api ${API_SRC_FILES})
target_link_libraries(ledger-core-bundle PUBLIC ledger-core-api)

# TODO: move all the following into a cmake module
if(IS_IOS GREATER_EQUAL 0)
  set(CMAKE_SHARED_LINKER_FLAGS "-Wall")
  set(FRAMEWORK_BUNDLE_IDENTIFIER "com.ledger.core")
  set(DEPLOYMENT_TARGET 9.0)
  set(DEVICE_FAMILY "1")
  set(PRODUCT_NAME ledger_core)
  set_target_properties(ledger-core-bundle PROPERTIES
    FRAMEWORK TRUE
    FRAMEWORK_VERSION A
    MACOSX_FRAMEWORK_IDENTIFIER ${FRAMEWORK_BUNDLE_IDENTIFIER}
    MACOSX_FRAMEWORK_BUNDLE_VERSION ${VERSION_MAJOR}
    MACOSX_FRAMEWORK_SHORT_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}"
    MACOSX_FRAMEWORK_INFO_PLIST ${CMAKE_BINARY_DIR}/framework.plist.in
    # "current version" in semantic format in Mach-O binary file
    VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
    # "compatibility version" in semantic format in Mach-O binary file
    SOVERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
    PUBLIC_HEADER "${CMAKE_BINARY_DIR}/include/ledger/core"
    XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET ${DEPLOYMENT_TARGET}
    XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY ${DEVICE_FAMILY}
    XCODE_ATTRIBUTE_SKIP_INSTALL "YES"
  )

  add_custom_command(
    TARGET ledger-core-bundle
    POST_BUILD
    COMMAND /bin/bash -c "${CMAKE_BINARY_DIR}/install_name.sh \${BUILT_PRODUCTS_DIR}/\${PRODUCT_NAME}.framework/\${PRODUCT_NAME} ${CMAKE_OSX_ARCHITECTURES}"
  )

  add_custom_command(
    TARGET ledger-core-bundle
    POST_BUILD
    COMMAND install_name_tool -id \"@rpath/\${PRODUCT_NAME}.framework/\${PRODUCT_NAME}\"
      \${BUILT_PRODUCTS_DIR}/\${PRODUCT_NAME}.framework/\${PRODUCT_NAME}
  )
endif()
