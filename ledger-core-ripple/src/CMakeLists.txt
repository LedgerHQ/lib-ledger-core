cmake_minimum_required(VERSION 3.12)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(ANDROID_CPP_FEATURES exceptions)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Automatically add API files to the library
file(
    GLOB ledger-core-ripple-api-sources
    "api/*"
)
list(APPEND ledger-core-ripple-sources ${ledger-core-ripple-api-sources})

# Configure JNI target
if (TARGET_JNI)
    message(STATUS "Configure project for JNI")

    file(
        GLOB ledger-core-ripple-jni-sources
        "jni/*.cpp"
        "jni/*.hpp"
        "jni/jni/*"
    )

    if(NOT ANDROID)
        find_package(JNI REQUIRED)
    endif()

    list(APPEND ledger-core-ripple-sources ${ledger-core-ripple-jni-sources})
    add_definitions(-DTARGET_JNI=1)
endif ()

link_directories(${CMAKE_BINARY_DIR}/lib)

# Add files to compile to the project
file(GLOB_RECURSE SRC_FILES *.cpp)
file(GLOB_RECURSE HEADERS_FILES *.h *.hpp)

# Interface library used to hold shared properties
add_library(ledger-core-ripple-interface INTERFACE)

target_compile_definitions(ledger-core-ripple-interface INTERFACE ledger_core_EXPORTS)
target_compile_definitions(ledger-core-ripple-interface INTERFACE
    LIB_VERSION_MAJOR=${VERSION_MAJOR}
    LIB_VERSION_MINOR=${VERSION_MINOR}
    LIB_VERSION_PATCH=${VERSION_PATCH}
)

string(FIND "${CMAKE_OSX_SYSROOT}" "iphone" IS_IOS)
if(IS_IOS GREATER_EQUAL 0)
    add_library(ledger-core-ripple SHARED
        ${ledger-core-ripple-sources}
        ${SRC_FILES}
        ${HEADERS_FILES})
    target_link_libraries(ledger-core-ripple PUBLIC ledger-core-ripple-interface)
    set(CMAKE_SHARED_LINKER_FLAGS "-Wall")
    set(FRAMEWORK_BUNDLE_IDENTIFIER "com.ledger.core")
    set(DEPLOYMENT_TARGET 9.0)
    set(DEVICE_FAMILY "1")
    set(PRODUCT_NAME ledger_core)
    set_target_properties(ledger-core-ripple PROPERTIES
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
        TARGET ledger-core-ripple
        POST_BUILD
        COMMAND /bin/bash -c "${CMAKE_BINARY_DIR}/install_name.sh \${BUILT_PRODUCTS_DIR}/\${PRODUCT_NAME}.framework/\${PRODUCT_NAME} ${CMAKE_OSX_ARCHITECTURES}"
    )
    add_custom_command(
        TARGET ledger-core-ripple
        POST_BUILD
        COMMAND install_name_tool -id \"@rpath/\${PRODUCT_NAME}.framework/\${PRODUCT_NAME}\"
        \${BUILT_PRODUCTS_DIR}/\${PRODUCT_NAME}.framework/\${PRODUCT_NAME}
    )
else()
    add_library(ledger-core-ripple-obj OBJECT
        ${ledger-core-ripple-sources}
        ${SRC_FILES}
        ${HEADERS_FILES})
    target_link_libraries(ledger-core-ripple-obj PUBLIC ledger-core-ripple-interface)

    # shared and static libraries built from the same object files
    add_library(ledger-core-ripple SHARED)
    target_link_libraries(ledger-core-ripple PUBLIC ledger-core-ripple-obj)

    add_library(ledger-core-ripple-static STATIC)
    target_link_libraries(ledger-core-ripple-static PUBLIC ledger-core-ripple-obj)
    install(TARGETS ledger-core-ripple-static DESTINATION "lib")
endif()

if(UNIX AND NOT APPLE AND NOT ANDROID)
    target_link_libraries(ledger-core-ripple-interface INTERFACE -static-libstdc++)
endif()

target_link_libraries(ledger-core-ripple-interface INTERFACE bigd)
if(MINGW OR MSVC)
    target_link_libraries(ledger-core-ripple-interface INTERFACE fmt-header-only)
else()
    target_link_libraries(ledger-core-ripple-interface INTERFACE fmt)
endif(MINGW OR MSVC)

target_link_libraries(ledger-core-ripple-interface INTERFACE crypto)
target_link_libraries(ledger-core-ripple-interface INTERFACE soci_sqlite3)
target_link_libraries(ledger-core-ripple-interface INTERFACE soci_core_static)
target_link_libraries(ledger-core-ripple-interface INTERFACE leveldb)

target_compile_definitions(ledger-core-ripple-interface INTERFACE SPDLOG_WCHAR_FILENAMES)
target_link_libraries(ledger-core-ripple-interface INTERFACE spdlog)


#Secp256k1
target_link_libraries(ledger-core-ripple-interface INTERFACE "${CMAKE_SOURCE_DIR}/core/lib/secp256k1/lib/${CMAKE_STATIC_LIBRARY_PREFIX}secp256k1${CMAKE_STATIC_LIBRARY_SUFFIX}")
link_directories("${CMAKE_SOURCE_DIR}/core/lib/secp256k1/lib")
add_dependencies(ledger-core-ripple-interface INTERFACE secp256k1)

target_link_libraries(ledger-core-ripple-interface INTERFACE ethash)

target_link_libraries(ledger-core-ripple-interface INTERFACE ${SQLITE_LIB})

file(
    GLOB LEDGER_CORE_API_HEADERS
    "api/*.hpp"
)

target_include_directories(ledger-core-ripple-interface INTERFACE ../lib/bigd)

if (TARGET_JNI)
    target_include_directories(ledger-core-ripple-interface INTERFACE ${JNI_INCLUDE_DIRS})
    target_link_libraries(ledger-core-ripple-interface INTERFACE ${JNI_LIBRARIES})
endif ()

target_include_directories(ledger-core-ripple-interface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(ledger-core-ripple-interface INTERFACE ../../ledger-core/src)
target_include_directories(ledger-core-ripple-interface INTERFACE ../../ledger-core/src/core/api)
target_include_directories(ledger-core-ripple-interface INTERFACE ../lib/bigd)
target_include_directories(ledger-core-ripple-interface INTERFACE ../lib/rapidjson/include)
target_include_directories(ledger-core-ripple-interface INTERFACE ../lib/soci/core)
target_include_directories(ledger-core-ripple-interface INTERFACE ../lib/soci_sqlite3)
target_include_directories(ledger-core-ripple-interface INTERFACE ../lib/openssl/include)
target_include_directories(ledger-core-ripple-interface INTERFACE ../lib/openssl/crypto/blake)
target_include_directories(ledger-core-ripple-interface INTERFACE ../lib/leveldb/include)
target_include_directories(ledger-core-ripple-interface INTERFACE ../lib/boost)
target_include_directories(ledger-core-ripple-interface INTERFACE ../lib/cereal/)
target_include_directories(ledger-core-ripple-interface INTERFACE ../lib/date/)
target_include_directories(ledger-core-ripple-interface INTERFACE ../lib/secp256k1)
target_include_directories(ledger-core-ripple-interface INTERFACE ../lib/ethash/src)

install(TARGETS ledger-core-ripple DESTINATION "lib")
install(FILES ${LEDGER_CORE_API_HEADERS} DESTINATION "include/ledger-core-ripple")
