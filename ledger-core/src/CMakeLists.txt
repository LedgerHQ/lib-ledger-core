cmake_minimum_required(VERSION 3.12)

# set(CMAKE_INCLUDE_CURRENT_DIR ON)
# set(ANDROID_CPP_FEATURES exceptions)

# Automatically add API files to the library
file(GLOB API_HEADER_FILES "${PROJECT_SOURCE_DIR}/inc/core/api/*.hpp")

# Configure JNI target
if (TARGET_JNI)
    message(STATUS "Configure project for JNI")

    file(GLOB JNI_SRC_FILES "${PROJECT_SOURCE_DIR}/inc/core/jni/*.cpp")
    file(GLOB JNI_HEADER_FILES "${PROJECT_SOURCE_DIR}/inc/core/jni/*.hpp")

    list(APPEND API_HEADER_FILES ${JNI_HEADER_FILES})
endif ()

# Add files to compile to the project
file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/core/*.cpp)
file(GLOB_RECURSE HEADERS_FILES ${PROJECT_SOURCE_DIR}/inc/core/*.hpp)

# The only correct way to remove multiple values from a list in cmake
list(FILTER HEADERS_FILES EXCLUDE REGEX ${PROJECT_SOURCE_DIR}/inc/core/api/*.hpp)

if (NOT PG_SUPPORT)
    list(REMOVE_ITEM SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/core/database/PostgreSQLBackend.cpp)
    list(REMOVE_ITEM HEADERS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/core/database/PostgreSQLBackend.hpp)
endif()

add_library(ledger-core-obj OBJECT ${SRC_FILES} ${HEADERS_FILES} ${API_SRC_FILES} ${API_HEADER_FILES})
add_library(Core::ledger-core-obj ALIAS ledger-core-obj)

target_compile_definitions(ledger-core-obj PUBLIC ledger_core_EXPORTS)
target_compile_definitions(ledger-core-obj PRIVATE
    LIB_VERSION_MAJOR=${VERSION_MAJOR}
    LIB_VERSION_MINOR=${VERSION_MINOR}
    LIB_VERSION_PATCH=${VERSION_PATCH}
)

# enable static analyzers if requested
include(StaticAnalyzers)
toggle_static_analyzers(ledger-core-obj)

# set compiler warnings
include(CompilerWarnings)
toggle_compiler_warnings(ledger-core-obj)

target_link_libraries(ledger-core-obj PUBLIC bigd)

if(MINGW OR MSVC)
    target_link_libraries(ledger-core-obj PUBLIC fmt-header-only)
else()
    target_link_libraries(ledger-core-obj PUBLIC fmt)
endif(MINGW OR MSVC)

# Crypto
if (SYS_OPENSSL)
    find_package(OpenSSL)
    target_link_libraries(ledger-core-obj PUBLIC OpenSSL::Crypto)
else()
    target_link_libraries(ledger-core-obj PUBLIC crypto)
endif()
target_link_libraries(ledger-core-obj PUBLIC blake)

# SSL
if(SSL_SUPPORT)
    if (SYS_OPENSSL)
        target_link_libraries(ledger-core-obj PUBLIC OpenSSL::SSL)
    else()
        target_link_libraries(ledger-core-obj PUBLIC ssl)
    endif()
endif()

target_link_libraries(ledger-core-obj PUBLIC soci_sqlite3)
target_link_libraries(ledger-core-obj PUBLIC soci_core_static)
target_link_libraries(ledger-core-obj PUBLIC leveldb)

if (PG_SUPPORT)
    target_link_libraries(ledger-core-obj PUBLIC soci_postgresql)
endif()

target_compile_definitions(ledger-core-obj PUBLIC SPDLOG_WCHAR_FILENAMES)
target_link_libraries(ledger-core-obj PUBLIC spdlog)

target_link_libraries(
  ledger-core-obj PUBLIC
  ${PROJECT_SOURCE_DIR}/lib/secp256k1/lib/${CMAKE_STATIC_LIBRARY_PREFIX}secp256k1${CMAKE_STATIC_LIBRARY_SUFFIX}
)

target_link_libraries(ledger-core-obj PUBLIC ethash)

target_link_libraries(ledger-core-obj PUBLIC ${SQLITE_LIB})

target_include_directories(
    ledger-core-obj SYSTEM PUBLIC
    ${PROJECT_SOURCE_DIR}/inc
    ${PROJECT_SOURCE_DIR}/lib/bigd
    ${PROJECT_SOURCE_DIR}/lib/rapidjson/include
    ${PROJECT_SOURCE_DIR}/lib/soci/core
    ${PROJECT_SOURCE_DIR}/lib/soci_sqlite3
    ${PROJECT_SOURCE_DIR}/lib/openssl/include
    ${PROJECT_SOURCE_DIR}/lib/blake
    ${PROJECT_SOURCE_DIR}/lib/leveldb/include
    ${PROJECT_SOURCE_DIR}/lib/boost
    ${PROJECT_SOURCE_DIR}/lib/cereal
    ${PROJECT_SOURCE_DIR}/lib/date
    ${PROJECT_SOURCE_DIR}/lib/secp256k1
    ${PROJECT_SOURCE_DIR}/lib/ethash/src
    ${PROJECT_SOURCE_DIR}/lib/ethash/src
    ${PROJECT_SOURCE_DIR}/lib/fmt/include
    ${PROJECT_SOURCE_DIR}/lib/spdlog/include
    # PG support
    $<$<BOOL:${PG_SUPPORT}>:${PROJECT_SOURCE_DIR}/lib/soci_postgresql>
)
