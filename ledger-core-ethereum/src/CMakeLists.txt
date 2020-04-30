cmake_minimum_required(VERSION 3.12)

# set(CMAKE_INCLUDE_CURRENT_DIR ON)
# set(ANDROID_CPP_FEATURES exceptions)

# Automatically add API files to the library
file(GLOB API_SRC_FILES "${PROJECT_SOURCE_DIR}/inc/ethereum/api/*.cpp")
file(GLOB API_HEADER_FILES "${PROJECT_SOURCE_DIR}/inc/ethereum/api/*.hpp")

# Configure JNI target
if (TARGET_JNI)
    file(GLOB JNI_SRC_FILES "${PROJECT_SOURCE_DIR}/inc/ethereum/jni/*.cpp")
    file(GLOB JNI_HEADER_FILES "${PROJECT_SOURCE_DIR}/inc/ethereum/jni/*.hpp")

    list(APPEND API_SRC_FILES ${JNI_SRC_FILES})
    list(APPEND API_HEADER_FILES ${JNI_HEADER_FILES})
endif ()

# Add files to compile to the project
file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ethereum/*.cpp)
file(GLOB_RECURSE HEADER_FILES ${PROJECT_SOURCE_DIR}/inc/ethereum/*.hpp)

add_library(ledger-core-ethereum-obj OBJECT ${SRC_FILES} ${HEADER_FILES} ${API_SRC_FILES} ${API_HEADER_FILES})
add_library(Core::ledger-core-ethereum-obj ALIAS ledger-core-ethereum-obj)

# enable static analyzers if requested
include(StaticAnalyzers)
toggle_static_analyzers(ledger-core-ethereum-obj)

# set compiler warnings
include(CompilerWarnings)
toggle_compiler_warnings(ledger-core-ethereum-obj)
disable_compiler_warnings_for_source_files("${API_SRC_FILES}")

# link the ledger-core library
target_link_libraries(ledger-core-ethereum-obj PRIVATE Core::ledger-core-obj)

target_include_directories(ledger-core-ethereum-obj
  PRIVATE ${PROJECT_SOURCE_DIR}/inc
  PUBLIC ${PROJECT_SOURCE_DIR}/inc/ethereum/api
)
