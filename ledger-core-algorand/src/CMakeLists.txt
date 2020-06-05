cmake_minimum_required(VERSION 3.12)

# set(CMAKE_INCLUDE_CURRENT_DIR ON)
# set(ANDROID_CPP_FEATURES exceptions)

# Automatically add API files to the library
file(GLOB API_HEADER_FILES "${PROJECT_SOURCE_DIR}/inc/algorand/api/*.hpp")

# Configure JNI target
if (TARGET_JNI)
    file(GLOB JNI_SRC_FILES "${PROJECT_SOURCE_DIR}/inc/algorand/jni/*.cpp")
    file(GLOB JNI_HEADER_FILES "${PROJECT_SOURCE_DIR}/inc/algorand/jni/*.hpp")

    list(APPEND API_HEADER_FILES ${JNI_HEADER_FILES})
endif ()

# Add files to compile to the project
file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/algorand/*.cpp)
file(GLOB_RECURSE HEADER_FILES ${PROJECT_SOURCE_DIR}/inc/algorand/*.hpp)

add_library(ledger-core-algorand-obj OBJECT ${SRC_FILES} ${HEADER_FILES} ${API_HEADER_FILES})

# enable static analyzers if requested
include(StaticAnalyzers)
toggle_static_analyzers(ledger-core-algorand-obj)

# set compiler warnings
include(CompilerWarnings)
toggle_compiler_warnings(ledger-core-algorand-obj)

# link the ledger-core objects
target_link_libraries(ledger-core-algorand-obj PRIVATE ledger-core-obj)

target_include_directories(ledger-core-algorand-obj PUBLIC
  ${PROJECT_SOURCE_DIR}/inc
  ${PROJECT_SOURCE_DIR}/inc/algorand/api
  ${PROJECT_SOURCE_DIR}/src/thirdparty/msgpack-c/include
)

