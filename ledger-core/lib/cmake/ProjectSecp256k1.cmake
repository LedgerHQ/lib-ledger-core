include(ExternalProject)

if (MSVC OR MINGW)
    set(_only_release_configuration -DCMAKE_CONFIGURATION_TYPES=Release)
    set(_overwrite_install_command INSTALL_COMMAND cmake --build <BINARY_DIR> --config Release --target install)
endif()

if (CMAKE_TOOLCHAIN_FILE)
    set(_toolchain_file_ -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE})
endif()

if (CMAKE_OSX_ARCHITECTURES)
    set(_osx_arch_ -DCMAKE_OSX_ARCHITECTURES:STRING=${CMAKE_OSX_ARCHITECTURES})
endif ()

if (CMAKE_OSX_SYSROOT)
    set(_osx_sysroot_ -DCMAKE_OSX_SYSROOT:STRING=${CMAKE_OSX_SYSROOT})
endif()

if (CMAKE_OSX_DEPLOYMENT_TARGET)
    set(_osx_deploy_target_ -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=${CMAKE_OSX_DEPLOYMENT_TARGET})
endif()

if (CMAKE_BUILD_TYPE)
    set(_build_type_ -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE})
endif()


set(prefix "${CMAKE_CURRENT_SOURCE_DIR}/secp256k1")

set(SECP256K1_LIBRARY "${CMAKE_BINARY_DIR}/core/lib/secp256k1/${CMAKE_STATIC_LIBRARY_PREFIX}secp256k1${CMAKE_STATIC_LIBRARY_SUFFIX}")
set(SECP256K1_INCLUDE_DIR "${prefix}/include")

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
        ${_toolchain_file_}
        ${_only_release_configuration}
        CMAKE_CACHE_ARGS ${_osx_arch_} ${_osx_sysroot_} ${_osx_deploy_target_} ${_build_type_}
        LOG_CONFIGURE 1
        LOG_INSTALL 1
        LOG_BUILD 1
        BUILD_COMMAND ""
        ${_overwrite_install_command}
        BUILD_BYPRODUCTS "${SECP256K1_LIBRARY}"
        BINARY_DIR ${CMAKE_BINARY_DIR}/core/lib/secp256k1
        STAMP_DIR ${CMAKE_BINARY_DIR}/core/lib/secp256k1
        TMP_DIR ${CMAKE_BINARY_DIR}/core/lib/secp256k1
)

# Create imported library
add_library(Secp256k1 STATIC IMPORTED GLOBAL)
file(MAKE_DIRECTORY "${SECP256K1_INCLUDE_DIR}")  # Must exist.
set_property(TARGET Secp256k1 PROPERTY IMPORTED_CONFIGURATIONS Release)
set_property(TARGET Secp256k1 PROPERTY IMPORTED_LOCATION_RELEASE "${SECP256K1_LIBRARY}")
set_property(TARGET Secp256k1 PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${SECP256K1_INCLUDE_DIR}")
add_dependencies(Secp256k1 secp256k1)
