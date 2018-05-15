cmake_minimum_required(VERSION 2.8.11)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

list(APPEND ledger-core-sources
        bytes/BytesReader.cpp
        ledger-core.h
        math/BigInt.h
        math/BigInt.cpp

        utils/endian.cpp
        utils/endian.h
        utils/hex.cpp
        utils/optional.hpp

        bytes/BytesWriter.cpp
        bytes/BytesWriter.h
        async/ExecutionContext.hpp
        async/Callback.hpp
        async/EventEmitter.hpp
        api/BigInt.hpp

        api_impl/LedgerCoreImpl.cpp
        api_impl/LedgerCoreImpl.hpp
        )

set(ANDROID_CPP_FEATURES exceptions)

# Automaticall ADD djinni api files to the library
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
file(GLOB ledger-core-djinni-api-sources
        "api/*"
        )
list(APPEND ledger-core-sources ${ledger-core-djinni-api-sources})

file(GLOB ledger-core-jni-sources
        "jni/*.cpp"
        "jni/*.hpp"
        "jni/jni/*"
        )

if (TARGET_JNI)
    message(STATUS "Configure project for JNI")
    find_package(JNI REQUIRED)
    list(APPEND ledger-core-sources ${ledger-core-jni-sources})
endif ()

link_directories(${CMAKE_BINARY_DIR}/lib)

set(LIB_BUILD_TYPE SHARED wallet/common/AbstractAddress.cpp wallet/common/AbstractAddress.h ledger-core.cpp)

file(GLOB_RECURSE SRC_FILES *.cpp)
file(GLOB_RECURSE HEADERS_FILES *.h *.hpp)
list(REMOVE_ITEM SRC_FILES ${ledger-core-jni-sources})
list(REMOVE_ITEM HEADERS_FILES ${ledger-core-jni-sources})

file(GLOB_RECURSE SRC_FILES *.cpp)
file(GLOB_RECURSE HEADERS_FILES *.h *.hpp)
list(REMOVE_ITEM SRC_FILES ${ledger-core-jni-sources})
list(REMOVE_ITEM HEADERS_FILES ${ledger-core-jni-sources})

add_library(
        ledger-core ${LIB_BUILD_TYPE}
        ${ledger-core-sources}
        ${SRC_FILES}
        ${HEADERS_FILES}
)

target_link_libraries(ledger-core bigd)
if(MINGW)
    target_link_libraries(ledger-core fmt-header-only)
else()
    target_link_libraries(ledger-core fmt)
endif(MINGW)
target_link_libraries(ledger-core crypto)
target_link_libraries(ledger-core sqlite3)
target_link_libraries(ledger-core soci_sqlite3)
target_link_libraries(ledger-core soci_core_static)
target_link_libraries(ledger-core leveldb)
target_link_libraries(ledger-core spdlog)
#Secp256k1
target_link_libraries(ledger-core "${CMAKE_SOURCE_DIR}/core/lib/secp256k1/lib/libsecp256k1.a")
link_directories("${CMAKE_SOURCE_DIR}/core/lib/secp256k1/lib")

string(FIND "${CMAKE_OSX_SYSROOT}" "iphone" IS_IOS)
if(IS_IOS GREATER_EQUAL 0)
    target_link_libraries(ledger-core $<TARGET_FILE:sqlite3>)
    link_directories($<TARGET_FILE_DIR:sqlite3>)
else()
    #SQlite3
    target_link_libraries(ledger-core "${CMAKE_BINARY_DIR}/core/lib/sqlite3/libsqlite3.a")
    link_directories("${CMAKE_BINARY_DIR}/core/lib/sqlite3")
endif()

file(GLOB_RECURSE LEDGER_CORE_HEADERS
        "*.hpp"
        "*.h"
        )

file(GLOB LEDGER_CORE_API_HEADERS
        "api/*.hpp"
        )

include_directories(ledger-core ../lib/bigd)

if (TARGET_JNI)
    include_directories(ledger-core ${JNI_INCLUDE_DIRS})
    target_link_libraries(ledger-core ${JNI_LIBRARIES})
endif ()

file(GLOB ledger-core-header
        "*"
        )
file(COPY ${ledger-core-header} DESTINATION ${CMAKE_BINARY_DIR}/include/ledger/core FILES_MATCHING PATTERN "*.h")
file(COPY ${ledger-core-header} DESTINATION ${CMAKE_BINARY_DIR}/include/ledger/core FILES_MATCHING PATTERN "*.hpp")

target_include_directories(ledger-core PUBLIC ..)
target_include_directories(ledger-core PUBLIC ../lib/bigd)
target_include_directories(ledger-core PUBLIC ../lib/fmt-3.0.0)
target_include_directories(ledger-core PUBLIC ../lib/rapidjson/include)
target_include_directories(ledger-core PUBLIC ../lib/soci/core)
target_include_directories(ledger-core PUBLIC ../lib/soci_sqlite3)
target_include_directories(ledger-core PUBLIC ../lib/sqlite3)
target_include_directories(ledger-core PUBLIC ../lib/spdlog/include)
target_include_directories(ledger-core PUBLIC ../lib/openssl/include)
target_include_directories(ledger-core PUBLIC ../lib/leveldb/include)
target_include_directories(ledger-core PUBLIC ../test/lib/boost)
target_include_directories(ledger-core PUBLIC ../lib/cereal/)
target_include_directories(ledger-core PUBLIC ../lib/date/)
target_include_directories(ledger-core PUBLIC ../lib/secp256k1)

target_compile_definitions(ledger-core PUBLIC LIB_VERSION_MAJOR=${VERSION_MAJOR} LIB_VERSION_MINOR=${VERSION_MINOR}
        LIB_VERSION_PATCH=${VERSION_PATCH})

install(TARGETS ledger-core DESTINATION "lib")
install(FILES ${LEDGER_CORE_API_HEADERS} DESTINATION "include/ledger-core")
