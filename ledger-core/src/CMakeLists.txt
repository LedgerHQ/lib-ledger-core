cmake_minimum_required(VERSION 3.12)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(ANDROID_CPP_FEATURES exceptions)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Automatically add API files to the library
file(
    GLOB ledger-core-api-sources
    "${CMAKE_SOURCE_DIR}/inc/core/api/*.cpp"
)
list(APPEND ledger-core-sources ${ledger-core-api-sources})

# Configure JNI target
if (TARGET_JNI)
    message(STATUS "Configure project for JNI")

    file(
        GLOB ledger-core-jni-sources
        "jni/*.cpp"
        "jni/*.hpp"
        "jni/jni/*"
    )

    if(NOT ANDROID)
        find_package(JNI REQUIRED)
    endif()

    list(APPEND ledger-core-sources ${ledger-core-jni-sources})
    add_definitions(-DTARGET_JNI=1)
endif ()

link_directories(${CMAKE_BINARY_DIR}/lib)

# Add files to compile to the project
file(GLOB_RECURSE SRC_FILES *.cpp)
file(GLOB_RECURSE HEADERS_FILES ../inc/*.{h,hpp})

if (NOT PG_SUPPORT)
    list(REMOVE_ITEM SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/core/database/PostgreSQLBackend.cpp)
    list(REMOVE_ITEM HEADERS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/core/database/PostgreSQLBackend.hpp)
endif()

# Interface library used to hold shared properties
add_library(ledger-core-interface INTERFACE)

target_compile_definitions(ledger-core-interface INTERFACE ledger_core_EXPORTS)
target_compile_definitions(ledger-core-interface INTERFACE
    LIB_VERSION_MAJOR=${VERSION_MAJOR}
    LIB_VERSION_MINOR=${VERSION_MINOR}
    LIB_VERSION_PATCH=${VERSION_PATCH}
)

string(FIND "${CMAKE_OSX_SYSROOT}" "iphone" IS_IOS)
if(IS_IOS GREATER_EQUAL 0)
    add_library(ledger-core SHARED
        ${ledger-core-sources}
        ${SRC_FILES}
        ${HEADERS_FILES})
    target_link_libraries(ledger-core PUBLIC ledger-core-interface)
    set(CMAKE_SHARED_LINKER_FLAGS "-Wall")
    set(FRAMEWORK_BUNDLE_IDENTIFIER "com.ledger.core")
    set(DEPLOYMENT_TARGET 9.0)
    set(DEVICE_FAMILY "1")
    set(PRODUCT_NAME ledger_core)
    set_target_properties(ledger-core PROPERTIES
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
        TARGET ledger-core
        POST_BUILD
        COMMAND /bin/bash -c "${CMAKE_BINARY_DIR}/install_name.sh \${BUILT_PRODUCTS_DIR}/\${PRODUCT_NAME}.framework/\${PRODUCT_NAME} ${CMAKE_OSX_ARCHITECTURES}"
    )
    add_custom_command(
        TARGET ledger-core
        POST_BUILD
        COMMAND install_name_tool -id \"@rpath/\${PRODUCT_NAME}.framework/\${PRODUCT_NAME}\"
        \${BUILT_PRODUCTS_DIR}/\${PRODUCT_NAME}.framework/\${PRODUCT_NAME}
    )
else()
    add_library(ledger-core-obj OBJECT
        ${ledger-core-sources}
        ${SRC_FILES}
        ${HEADERS_FILES})
    target_link_libraries(ledger-core-obj PUBLIC ledger-core-interface)

    # shared and static libraries built from the same object files
    add_library(ledger-core SHARED)
    target_link_libraries(ledger-core PUBLIC ledger-core-obj)

    add_library(ledger-core-static STATIC)
    target_link_libraries(ledger-core-static PUBLIC ledger-core-obj)
endif()

if(UNIX AND NOT APPLE AND NOT ANDROID)
    target_link_libraries(ledger-core-interface INTERFACE -static-libstdc++)
endif()

target_link_libraries(ledger-core-interface INTERFACE bigd)
if(MINGW OR MSVC)
    target_link_libraries(ledger-core-interface INTERFACE fmt-header-only)
else()
    target_link_libraries(ledger-core-interface INTERFACE fmt)
endif(MINGW OR MSVC)

# Crypto
if (SYS_OPENSSL)
    find_package(OpenSSL)
    target_link_libraries(ledger-core-interface INTERFACE OpenSSL::Crypto)
else()
    target_link_libraries(ledger-core-interface INTERFACE crypto)
endif()
target_link_libraries(ledger-core-interface INTERFACE blake)

# SSL
if(SSL_SUPPORT)
    if (SYS_OPENSSL)
        target_link_libraries(ledger-core-interface INTERFACE OpenSSL::SSL)
    else()
        target_link_libraries(ledger-core-interface INTERFACE ssl)
    endif()
endif()

target_link_libraries(ledger-core-interface INTERFACE soci_sqlite3)
target_link_libraries(ledger-core-interface INTERFACE soci_core_static)
target_link_libraries(ledger-core-interface INTERFACE leveldb)

if (PG_SUPPORT)
    target_link_libraries(ledger-core-interface INTERFACE soci_postgresql)
endif()

target_compile_definitions(ledger-core-interface INTERFACE SPDLOG_WCHAR_FILENAMES)
target_link_libraries(ledger-core-interface INTERFACE spdlog)

include(GNUInstallDirs)

target_link_libraries(
    ledger-core-interface INTERFACE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/lib/secp256k1/lib/${CMAKE_STATIC_LIBRARY_PREFIX}secp256k1${CMAKE_STATIC_LIBRARY_SUFFIX}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_FULL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}secp256k1${CMAKE_STATIC_LIBRARY_SUFFIX}>
)

target_link_libraries(ledger-core-interface INTERFACE ethash)

target_link_libraries(ledger-core-interface INTERFACE ${SQLITE_LIB})

target_include_directories(
    ledger-core-interface INTERFACE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/inc>                      $<INSTALL_INTERFACE:include/ledger-core>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/lib/bigd>                 $<INSTALL_INTERFACE:include/ledger-core/lib/bigd>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/lib/rapidjson/include>    $<INSTALL_INTERFACE:include/ledger-core/lib/rapidjson/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/lib/soci/core>            $<INSTALL_INTERFACE:include/ledger-core/lib/soci/core>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/lib/soci_sqlite3>         $<INSTALL_INTERFACE:include/ledger-core/lib/soci_sqlite3>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/lib/openssl/include>      $<INSTALL_INTERFACE:include/ledger-core/lib/openssl/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/lib/blake>                $<INSTALL_INTERFACE:include/ledger-core/lib/blake>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/lib/leveldb/include>      $<INSTALL_INTERFACE:include/ledger-core/lib/leveldb/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/lib/boost>                $<INSTALL_INTERFACE:include/ledger-core/lib/boost>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/lib/cereal>               $<INSTALL_INTERFACE:include/ledger-core/lib/cereal>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/lib/date>                 $<INSTALL_INTERFACE:include/ledger-core/lib/date>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/lib/secp256k1>            $<INSTALL_INTERFACE:include/ledger-core/lib/secp256k1>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/lib/ethash/src>           $<INSTALL_INTERFACE:include/ledger-core/lib/ethash/src>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/lib/ethash/src>           $<INSTALL_INTERFACE:include/ledger-core/lib/ethash/src>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/lib/fmt/include>          $<INSTALL_INTERFACE:include/ledger-core/lib/fmt/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/lib/spdlog/include>       $<INSTALL_INTERFACE:include/ledger-core/lib/spdlog/include>

    # PG support
    $<$<BOOL:${PG_SUPPORT}>:$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/lib/soci_postgresql>>

    # Hack: work around to find the ledger-core API header files in dependent projects
    $<INSTALL_INTERFACE:include/ledger-core/core/api>
)

if (TARGET_JNI)
    target_include_directories(ledger-core-interface INTERFACE ${JNI_INCLUDE_DIRS})
    target_link_libraries(ledger-core-interface INTERFACE ${JNI_LIBRARIES})
endif ()
