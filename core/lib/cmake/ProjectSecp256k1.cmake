include(ExternalProject)

if (MSVC)
    set(_only_release_configuration -DCMAKE_CONFIGURATION_TYPES=Release)
    set(_overwrite_install_command INSTALL_COMMAND cmake --build <BINARY_DIR> --config Release --target install)
endif()

#message(STATUS "*************************")
#message(STATUS "CMAKE_PLATFORM = ${CMAKE_PLATFORM}")
#message(STATUS "CMAKE_OSX_SYSROOT = ${CMAKE_OSX_SYSROOT}")
#message(STATUS "MACOSX_BUNDLE_GUI_IDENTIFIER = ${MACOSX_BUNDLE_GUI_IDENTIFIER}")
#message(STATUS "*************************")
#
#set(_build_command_ BUILD_COMMAND "")
#if("${CMAKE_PLATFORM}" STREQUAL "IOS")
#    set(CMAKE_XCODE_ATTRIBUTE_SUPPORTED_PLATFORMS "macosx iphonesimulator")
#    set(CMAKE_OSX_SYSROOT "iphonesimulator" CACHE STRING "System root for MAcOS" FORCE)
#    set(CMAKE_MACOSX_BUNDLE "YES" CACHE STRING "bundle macos" FORCE)
#    message(STATUS "*************************")
#    message(STATUS "*************************")
#    message(STATUS "*************************")
#  #-DCMAKE_MACOSX_BUNDLE=YES
#  #-DCMAKE_OSX_SYSROOT="iphoneos"
#endif()

set(prefix "${CMAKE_CURRENT_SOURCE_DIR}/secp256k1")

set(SECP256K1_LIBRARY "${CMAKE_BINARY_DIR}/core/lib/secp256k1/${CMAKE_STATIC_LIBRARY_PREFIX}secp256k1${CMAKE_STATIC_LIBRARY_SUFFIX}")
set(SECP256K1_INCLUDE_DIR "${prefix}/include")

if("${CMAKE_PLATFORM}" STREQUAL "IOS")
    #set(XCODE_ATTRIBUTE_WRAPPER_EXTENSION "application" CACHE STRING "bundle extension" FORCE)
    #set(CMAKE_MACOSX_BUNDLE YES CACHE BOOL "bundle as applciation" FORCE)
    message(STATUS "*************************")
    message(STATUS "CMAKE_MACOSX_BUNDLE = ${CMAKE_MACOSX_BUNDLE}")
    message(STATUS "MACOSX_BUNDLE_GUI_IDENTIFIER = ${MACOSX_BUNDLE_GUI_IDENTIFIER}")
    message(STATUS "*************************")
ExternalProject_Add(
        secp256k1
        PREFIX "${prefix}"
        DOWNLOAD_NAME secp256k1-ac8ccf29.tar.gz
        DOWNLOAD_NO_PROGRESS 1
        URL https://github.com/chfast/secp256k1/archive/ac8ccf29b8c6b2b793bc734661ce43d1f952977a.tar.gz
        URL_HASH SHA256=02f8f05c9e9d2badc91be8e229a07ad5e4984c1e77193d6b00e549df129e7c3a
        PATCH_COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_LIST_DIR}/../secp256k1/CMakeLists.txt <SOURCE_DIR>
        CMAKE_ARGS -DCMAKE_MACOSX_BUNDLE_GUI_IDENTIFIER:STRING=${MACOSX_BUNDLE_GUI_IDENTIFIER} -DCMAKE_MACOSX_BUNDLE:BOOL=ON -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DCMAKE_POSITION_INDEPENDENT_CODE=${BUILD_SHARED_LIBS} -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        LOG_CONFIGURE 1
        LOG_DOWNLOAD 1
        LOG_BUILD 1
        LOG_INSTALL 1
        BUILD_BYPRODUCTS "${SECP256K1_LIBRARY}"
)
else()
ExternalProject_Add(
        secp256k1
        PREFIX "${prefix}"
        DOWNLOAD_NAME secp256k1-ac8ccf29.tar.gz
        DOWNLOAD_NO_PROGRESS 1
        URL https://github.com/chfast/secp256k1/archive/ac8ccf29b8c6b2b793bc734661ce43d1f952977a.tar.gz
        URL_HASH SHA256=02f8f05c9e9d2badc91be8e229a07ad5e4984c1e77193d6b00e549df129e7c3a
        PATCH_COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_LIST_DIR}/../secp256k1/CMakeLists.txt <SOURCE_DIR>
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DCMAKE_POSITION_INDEPENDENT_CODE=${BUILD_SHARED_LIBS}
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        ${_only_release_configuration}
        LOG_CONFIGURE 1
        BUILD_COMMAND ""
        ${_overwrite_install_command}
        LOG_INSTALL 1
        BUILD_BYPRODUCTS "${SECP256K1_LIBRARY}"
)
endif()
# Create imported library
add_library(Secp256k1 STATIC IMPORTED)
file(MAKE_DIRECTORY "${SECP256K1_INCLUDE_DIR}")  # Must exist.
set_property(TARGET Secp256k1 PROPERTY IMPORTED_CONFIGURATIONS Release)
set_property(TARGET Secp256k1 PROPERTY IMPORTED_LOCATION_RELEASE "${SECP256K1_LIBRARY}")
set_property(TARGET Secp256k1 PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${SECP256K1_INCLUDE_DIR}")
set_property(TARGET Secp256k1 PROPERTY MACOSX_BUNDLE TRUE)
set_property(TARGET secp256k1 PROPERTY MACOSX_BUNDLE TRUE)
add_dependencies(Secp256k1 secp256k1)
