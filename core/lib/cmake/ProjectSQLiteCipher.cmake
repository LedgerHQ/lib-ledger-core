project(sqlcipher)
include(ExternalProject)


set(OPENSSL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/openssl")
set(OPENSSL_LIB $<TARGET_FILE:crypto>)
string(FIND "${CMAKE_OSX_SYSROOT}" "iphone" IS_IOS)
if(IS_IOS GREATER_EQUAL 0)
    set(OPENSSL_LIB ${CMAKE_BINARY_DIR}/core/lib/openssl/crypto/${CMAKE_BUILD_TYPE}-${CMAKE_OSX_SYSROOT})
endif()

set(_ld_flags_ -L${OPENSSL_LIB})
set(_c_flags_ "-DSQLITE_HAS_CODEC -I${OPENSSL_DIR}/include -L${OPENSSL_LIB}")
set(_configure_options_ --with-pic --disable-shared --with-crypto-lib=none --disable-tcl --enable-tempstore=yes )
set(_build_command_ ${CMAKE_MAKE_PROGRAM})

set(prefix "${CMAKE_CURRENT_SOURCE_DIR}/sqlcipher")

if(IS_IOS GREATER_EQUAL 0)
    file(MAKE_DIRECTORY "${prefix}/lib")  # Must exist.

    find_program(XCODE-SELECT xcode-select)
    if ("${XCODE-SELECT}" MATCHES "-NOTFOUND")
        message(FATAL_ERROR "The xcode-select program could not be found.")
    endif()

    execute_process(COMMAND ${XCODE-SELECT} -print-path
            OUTPUT_VARIABLE XCODE_DEVELOPER_DIR
            OUTPUT_STRIP_TRAILING_WHITESPACE
            )

    if (CMAKE_OSX_ARCHITECTURES STREQUAL "x86_64")
        set(OS_COMPILER "iPhoneSimulator")
        set(_host_ "x86_64-apple-darwin")
    else()
        set(OS_COMPILER "iPhoneOS")
        set(_host_ "arm-apple-darwin")
    endif()

    set(CROSS_TOP "${XCODE_DEVELOPER_DIR}/Platforms/${OS_COMPILER}.platform/Developer")
    set(CROSS_SDK "${OS_COMPILER}.sdk")
    #set(_c_flags_ "-arch ${CMAKE_OSX_ARCHITECTURES} -isysroot ${CROSS_TOP}/SDKs/${CROSS_SDK} ${_c_flags_}")

    set(_configure_options_ ${_configure_options_} --host ${_host_})
    #set(_c_flags_ "${_c_flags_} -arch ${CMAKE_OSX_ARCHITECTURES} -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator.sdk")
    set(_c_flags_ "${_c_flags_} -arch ${CMAKE_OSX_ARCHITECTURES} -isysroot ${CROSS_TOP}/SDKs/${CROSS_SDK}")
    set(_build_command_ bash ${CMAKE_CURRENT_SOURCE_DIR}/cmake/build_sqlcipher_ios.sh)
    set(_overwrite_install_command INSTALL_COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/cmake/install_sqlcipher_ios.sh)
endif()

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

set(SQLCIPHER_LIB "${prefix}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}sqlcipher${CMAKE_STATIC_LIBRARY_SUFFIX}")

ExternalProject_Add(
        SQLCipher
        DEPENDS crypto
        PREFIX "${prefix}"
        DOWNLOAD_NO_PROGRESS 1
        GIT_REPOSITORY https://github.com/SQLCipher/SQLCipher.git
        UPDATE_COMMAND ""
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        ${_toolchain_file_}
        ${_only_release_configuration}
        LOG_CONFIGURE 1
        LOG_INSTALL 1
        LOG_BUILD 1
        CONFIGURE_COMMAND env CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER} CPPFLAGS=-I${OPENSSL_DIR}/include <SOURCE_DIR>/configure ${_configure_options_} CFLAGS=${_c_flags_} LDFLAGS=${_ld_flags_} LIBS=-lcrypto --prefix=${prefix}
        BUILD_IN_SOURCE 1
        BUILD_COMMAND ${_build_command_}
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