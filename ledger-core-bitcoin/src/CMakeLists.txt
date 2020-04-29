cmake_minimum_required(VERSION 3.12)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(ANDROID_CPP_FEATURES exceptions)

# Automatically add API files to the library
file(GLOB API_SRC_FILES "${PROJECT_SOURCE_DIR}/inc/bitcoin/api/*.cpp")
file(GLOB API_HEADER_FILES "${PROJECT_SOURCE_DIR}/inc/bitcoin/api/*.hpp")

# Configure JNI target
if (TARGET_JNI)
    message(STATUS "Configure project for JNI")

    file(GLOB JNI_SRC_FILES "${PROJECT_SOURCE_DIR}/inc/core/jni/*.cpp")
    file(GLOB JNI_HEADER_FILES "${PROJECT_SOURCE_DIR}/inc/core/jni/*.hpp")

    list(APPEND API_SRC_FILES ${JNI_SRC_FILES})
    list(APPEND API_HEADER_FILES ${JNI_HEADER_FILES})
endif ()

# Add files to compile to the project
file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/bitcoin/*.cpp)
file(GLOB_RECURSE HEADER_FILES ${PROJECT_SOURCE_DIR}/inc/bitcoin/*.hpp)

# Interface library used to hold shared properties
add_library(ledger-core-bitcoin-interface INTERFACE)

add_library(ledger-core-bitcoin-obj OBJECT ${SRC_FILES} ${HEADER_FILES} ${API_SRC_FILES} ${API_HEADER_FILES})
target_link_libraries(ledger-core-bitcoin-obj PUBLIC ledger-core-bitcoin-interface)
add_library(Core::ledger-core-bitcoin-obj ALIAS ledger-core-bitcoin-obj)

# enable static analyzers if requested
include(StaticAnalyzers)
toggle_static_analyzers(ledger-core-bitcoin-obj)

# set compiler warnings
include(CompilerWarnings)
toggle_compiler_warnings(ledger-core-bitcoin-obj)
disable_compiler_warnings_for_source_files("${API_SRC_FILES}")

if(UNIX AND NOT APPLE AND NOT ANDROID)
    target_link_libraries(ledger-core-bitcoin-interface INTERFACE -static-libstdc++)
endif()

target_link_libraries(ledger-core-bitcoin-interface INTERFACE Core::ledger-core-obj)

target_include_directories(ledger-core-bitcoin-interface INTERFACE ${PROJECT_SOURCE_DIR}/inc)
target_include_directories(ledger-core-bitcoin-interface INTERFACE ${PROJECT_SOURCE_DIR}/inc/bitcoin/api)

