cmake_minimum_required(VERSION 3.12)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(ANDROID_CPP_FEATURES exceptions)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(ledger-core 0.1.0 REQUIRED)

# Automatically add API files to the library
file(
    GLOB ledger-core-algorand-api-sources
    "${CMAKE_SOURCE_DIR}/inc/algorand/api/*"
)
list(APPEND ledger-core-algorand-sources ${ledger-core-algorand-api-sources})

# Configure JNI target
if (TARGET_JNI)
    message(STATUS "Configure project for JNI")

    file(
        GLOB ledger-core-algorand-jni-sources
        "jni/*.cpp"
        "jni/*.hpp"
        "jni/jni/*"
    )

    if(NOT ANDROID)
        find_package(JNI REQUIRED)
    endif()

    list(APPEND ledger-core-algorand-sources ${ledger-core-algorand-jni-sources})
    add_definitions(-DTARGET_JNI=1)
endif ()

# Add files to compile to the project
file(GLOB_RECURSE SRC_FILES *.cpp)

# Interface library used to hold shared properties
add_library(ledger-core-algorand-interface INTERFACE)

target_compile_definitions(ledger-core-algorand-interface INTERFACE ledger_core_EXPORTS)
target_compile_definitions(ledger-core-algorand-interface INTERFACE
    LIB_VERSION_MAJOR=${VERSION_MAJOR}
    LIB_VERSION_MINOR=${VERSION_MINOR}
    LIB_VERSION_PATCH=${VERSION_PATCH}
)

string(FIND "${CMAKE_OSX_SYSROOT}" "iphone" IS_IOS)
if(IS_IOS GREATER_EQUAL 0)
    add_library(ledger-core-algorand SHARED
        ${ledger-core-algorand-sources}
        ${SRC_FILES}
        ${CORE_SRC_FILES}
        ${HEADERS_FILES})
    target_link_libraries(ledger-core-algorand PUBLIC ledger-core-algorand-interface)
    set(CMAKE_SHARED_LINKER_FLAGS "-Wall")
    set(FRAMEWORK_BUNDLE_IDENTIFIER "com.ledger.core")
    set(DEPLOYMENT_TARGET 9.0)
    set(DEVICE_FAMILY "1")
    set(PRODUCT_NAME ledger_core)
    set_target_properties(ledger-core-algorand PROPERTIES
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
        TARGET ledger-core-algorand
        POST_BUILD
        COMMAND /bin/bash -c "${CMAKE_BINARY_DIR}/install_name.sh \${BUILT_PRODUCTS_DIR}/\${PRODUCT_NAME}.framework/\${PRODUCT_NAME} ${CMAKE_OSX_ARCHITECTURES}"
    )
    add_custom_command(
        TARGET ledger-core-algorand
        POST_BUILD
        COMMAND install_name_tool -id \"@rpath/\${PRODUCT_NAME}.framework/\${PRODUCT_NAME}\"
        \${BUILT_PRODUCTS_DIR}/\${PRODUCT_NAME}.framework/\${PRODUCT_NAME}
    )
else()
    add_library(ledger-core-algorand-obj OBJECT
        ${ledger-core-algorand-sources}
        ${SRC_FILES}
        ${CORE_SRC_FILES}
        ${HEADERS_FILES}
        ${CORE_HEADER_FILES})
    target_link_libraries(ledger-core-algorand-obj PUBLIC ledger-core-algorand-interface)

    # shared and static libraries built from the same object files
    add_library(ledger-core-algorand SHARED)
    target_link_libraries(ledger-core-algorand PUBLIC ledger-core-algorand-obj)

    add_library(ledger-core-algorand-static STATIC)
    target_link_libraries(ledger-core-algorand-static PUBLIC ledger-core-algorand-obj)
    install(TARGETS ledger-core-algorand-static DESTINATION "lib")
endif()

if(UNIX AND NOT APPLE AND NOT ANDROID)
    target_link_libraries(ledger-core-algorand-interface INTERFACE -static-libstdc++)
endif()

# link the ledger-core library
target_link_libraries(ledger-core-algorand-interface INTERFACE Core::ledger-core)

if (TARGET_JNI)
    target_include_directories(ledger-core-algorand-interface INTERFACE ${JNI_INCLUDE_DIRS})
    target_link_libraries(ledger-core-algorand-interface INTERFACE ${JNI_LIBRARIES})
endif ()

target_include_directories(ledger-core-algorand-interface INTERFACE ${CMAKE_SOURCE_DIR}/inc)
target_include_directories(ledger-core-algorand-interface INTERFACE ${CMAKE_SOURCE_DIR}/inc/algorand/api)

install(TARGETS ledger-core-algorand DESTINATION lib)
