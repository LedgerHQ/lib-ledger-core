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
set(_libs_ -lcrypto)
set(prefix "${CMAKE_CURRENT_SOURCE_DIR}/sqlcipher")

if(IS_IOS GREATER_EQUAL 0)
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
        set(SKIP_BUILD_SQLCIPHER ON)
    endif()

    set(CROSS_TOP "${XCODE_DEVELOPER_DIR}/Platforms/${OS_COMPILER}.platform/Developer")
    set(CROSS_SDK "${OS_COMPILER}.sdk")

    set(_configure_options_ ${_configure_options_} --host=${_host_})
    set(_c_flags_ "${_c_flags_} -arch ${CMAKE_OSX_ARCHITECTURES} -isysroot ${CROSS_TOP}/SDKs/${CROSS_SDK}")
    set(_build_command_ bash ${CMAKE_CURRENT_SOURCE_DIR}/cmake/build_sqlcipher_ios.sh)
    set(_overwrite_install_command INSTALL_COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/cmake/install_sqlcipher.sh)
elseif(ANDROID)
    string(FIND ${CMAKE_TOOLCHAIN_FILE} "armeabi" IS_ARM)
    string(FIND ${CMAKE_TOOLCHAIN_FILE} "arm64" IS_ARCH)
    if (IS_ARM GREATER_EQUAL 0)
        set(_toolchain_ arm-linux-androideabi)
    elseif(IS_ARCH GREATER_EQUAL 0)
        set(_toolchain_ aarch64-linux-android)
    else()
        set(_toolchain_ i686-linux-android)
    endif()
    set(_overwrite_install_command INSTALL_COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/cmake/install_sqlcipher.sh)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(_libs_ "-lcrypto -ldl -lm -lpthread")
    set(_build_command_ bash ${CMAKE_CURRENT_SOURCE_DIR}/cmake/build_sqlcipher.sh)
    set(_overwrite_install_command INSTALL_COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/cmake/install_sqlcipher.sh)
elseif (MSVC OR MINGW)
    set(_configure_options_ ${_configure_options_} --build=x86_64 --host=x86_64-w64-mingw32 --target=x86_64-w64-mingw32)
    set(_only_release_configuration -DCMAKE_CONFIGURATION_TYPES=Release)
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
if (NOT DEFINED SKIP_BUILD_SQLCIPHER)
    if(ANDROID)
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
                CONFIGURE_COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/cmake/configure_sqlcipher_android.sh ${CMAKE_CURRENT_SOURCE_DIR} ${_toolchain_} $ENV{ANDROID_NDK_r16b} ${OPENSSL_DIR} ${CMAKE_BINARY_DIR}/core/lib/openssl/crypto
                BINARY_DIR ${prefix}/src/SQLCipher
                BUILD_COMMAND ${_build_command_}
                BUILD_BYPRODUCTS "${SQLCIPHER_LIB}"
                ${_overwrite_install_command}
        )
    else()
        ExternalProject_Add(
                SQLCipher
                DEPENDS crypto
                PREFIX "${prefix}"
                DOWNLOAD_NO_PROGRESS 1
                GIT_REPOSITORY https://github.com/SQLCipher/SQLCipher.git
                GIT_SUBMODULES ""
                UPDATE_COMMAND ""
                CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
                ${_toolchain_file_}
                ${_only_release_configuration}
                LOG_CONFIGURE 1
                LOG_INSTALL 1
                LOG_BUILD 1
                CONFIGURE_COMMAND env CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER} CPPFLAGS=-I${OPENSSL_DIR}/include <SOURCE_DIR>/configure ${_configure_options_} CFLAGS=${_c_flags_} LDFLAGS=${_ld_flags_} LIBS=${_libs_} --prefix=${prefix}
                BUILD_COMMAND ${_build_command_}
                BUILD_BYPRODUCTS "${SQLCIPHER_LIB}"
                ${_overwrite_install_command}
        )
    endif()

else()
    #WARNING: for iOS arm64 and armv7, sqlcipher should be built before running
    #project build (launch build_sqlcipher_ios_arm64_armv7.sh)
    ExternalProject_Add(
            SQLCipher
            DEPENDS crypto
            PREFIX "${prefix}"
            DOWNLOAD_NO_PROGRESS 1
            GIT_REPOSITORY https://github.com/SQLCipher/SQLCipher.git
            UPDATE_COMMAND ""
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ""
            INSTALL_COMMAND ""
            COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/cmake/patch_sqlcipher_ios.sh ${CMAKE_OSX_ARCHITECTURES} ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()


set(SQLCIPHER_INCLUDE_DIR "${prefix}/include")

# Create imported library
add_library(sqlcipher STATIC IMPORTED)
file(MAKE_DIRECTORY "${SQLCIPHER_INCLUDE_DIR}")  # Must exist.
set_property(TARGET sqlcipher PROPERTY IMPORTED_CONFIGURATIONS Release)
set_property(TARGET sqlcipher PROPERTY IMPORTED_LOCATION_RELEASE "${SQLCIPHER_LIB}")

set_property(TARGET sqlcipher PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${SQLCIPHER_INCLUDE_DIR}")
add_dependencies(sqlcipher SQLCipher)
export(PACKAGE SQLCipher)