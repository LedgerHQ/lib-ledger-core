project(sqlcipher)
include(ExternalProject)


set(OPENSSL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/openssl")
set(OPENSSL_LIB $<TARGET_FILE:crypto>)
set(_ld_flags_ -L${OPENSSL_LIB})
set(_c_flags_ "-DSQLITE_HAS_CODEC -I${OPENSSL_DIR}/include -L${OPENSSL_LIB}")
set(_configure_options_ --with-pic --disable-shared --with-crypto-lib=none --disable-tcl --enable-tempstore=yes )
set(prefix "${CMAKE_CURRENT_SOURCE_DIR}/sqlcipher")

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

if (CMAKE_BUILD_TYPE)
    set(_build_type_ -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE})
endif()

set(SQLCIPHER_LIB "${prefix}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}sqlcipher{CMAKE_STATIC_LIBRARY_SUFFIX}")

ExternalProject_Add(
        SQLCipher
        DEPENDS crypto
        PREFIX "${prefix}"
        DOWNLOAD_NO_PROGRESS 1
        GIT_REPOSITORY https://github.com/SQLCipher/SQLCipher.git
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        ${_toolchain_file_}
        ${_only_release_configuration}
        LOG_CONFIGURE 1
        LOG_INSTALL 1
        LOG_BUILD 1
        CONFIGURE_COMMAND env CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER} CPPFLAGS=-I${OPENSSL_DIR}/include <SOURCE_DIR>/configure ${_configure_options_} CFLAGS=${_c_flags_} LDFLAGS=${_ld_flags_} LIBS=-lcrypto --prefix=${prefix}
        BUILD_IN_SOURCE 1
        BUILD_COMMAND ${CMAKE_MAKE_PROGRAM}
        BUILD_BYPRODUCTS "${SQLCIPHER_LIB}"
        ${_overwrite_install_command}
)

set(SQLCIPHER_INCLUDE_DIR "${prefix}/include")

# Create imported library
add_library(sqlcipher STATIC IMPORTED)
file(MAKE_DIRECTORY "${SQLCIPHER_INCLUDE_DIR}")  # Must exist.
set_property(TARGET sqlcipher PROPERTY IMPORTED_CONFIGURATIONS Release)
set_property(TARGET sqlcipher PROPERTY IMPORTED_LOCATION_RELEASE "${SQLCIPHER_LIB}")

set_property(TARGET sqlcipher PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${SQLCIPHER_INCLUDE_DIR}")
add_dependencies(sqlcipher SQLCipher)
export(PACKAGE SQLCipher)