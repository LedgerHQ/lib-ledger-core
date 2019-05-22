cmake_minimum_required(VERSION 3.12)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(ANDROID_CPP_FEATURES exceptions)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Automatically add API files to the library
file(
    GLOB ledger-core-ripple-api-sources
    "${CMAKE_SOURCE_DIR}/inc/api/*"
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

#link_directories(${CMAKE_BINARY_DIR}/lib)

# Add files to compile to the project
file(GLOB_RECURSE SRC_FILES *.cpp)

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
        ${CORE_SRC_FILES}
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
        ${CORE_SRC_FILES}
        ${HEADERS_FILES}
        ${CORE_HEADER_FILES})
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

# link the ledger-core library
target_link_directories(ledger-core-ripple-interface INTERFACE ${CMAKE_SOURCE_DIR}/../ledger-core/build/src)
target_link_libraries(ledger-core-ripple-interface INTERFACE ledger-core)

if (TARGET_JNI)
    target_include_directories(ledger-core-ripple-interface INTERFACE ${JNI_INCLUDE_DIRS})
    target_link_libraries(ledger-core-ripple-interface INTERFACE ${JNI_LIBRARIES})
endif ()

target_include_directories(ledger-core-ripple-interface INTERFACE ${CMAKE_SOURCE_DIR}/inc)
target_include_directories(ledger-core-ripple-interface INTERFACE "${CMAKE_SOURCE_DIR}/../ledger-core/inc")
target_include_directories(ledger-core-ripple-interface INTERFACE "${CMAKE_SOURCE_DIR}/lib/bigd")
target_include_directories(ledger-core-ripple-interface INTERFACE "${CMAKE_SOURCE_DIR}/lib/boost")
target_include_directories(ledger-core-ripple-interface INTERFACE "${CMAKE_SOURCE_DIR}/lib/cereal")
target_include_directories(ledger-core-ripple-interface INTERFACE "${CMAKE_SOURCE_DIR}/lib/fmt/include")

install(TARGETS ledger-core-ripple DESTINATION "lib")
