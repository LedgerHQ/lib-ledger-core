cmake_minimum_required(VERSION 3.12)

if(MSVC)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif(MSVC)

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
#    if(ANDROID)
#        if (UNIX)
#            set( JAVA_AWT_INCLUDE_PATH $ENV{JAVA_HOME}/include CACHE PATH "" FORCE)
#            list(APPEND JAVA_INCLUDE_PATH2 $ENV{JAVA_HOME}/include)
#            if(ANDROID_OSX)
#                list(APPEND JAVA_INCLUDE_PATH2 $ENV{JAVA_HOME}/include/darwin)
#                list(APPEND JAVA_AWT_LIBRARY $ENV{JAVA_HOME}/jre/lib/libjawt.dylib)
#                list(APPEND JAVA_JVM_LIBRARY $ENV{JAVA_HOME}/jre/lib/server/libjvm.dylib)
#            else()
#                list(APPEND JAVA_INCLUDE_PATH2 $ENV{JAVA_HOME}/include/linux)
#                list(APPEND JAVA_AWT_LIBRARY $ENV{JAVA_HOME}/jre/lib/i386/libjawt.so)
#                list(APPEND JAVA_JVM_LIBRARY $ENV{JAVA_HOME}/jre/lib/i386/server/libjvm.so)
#            endif()
#        endif ()
#    endif()
    if(NOT ANDROID)
        find_package(JNI REQUIRED)
    endif()
    list(APPEND ledger-core-sources ${ledger-core-jni-sources})
endif ()

link_directories(${CMAKE_BINARY_DIR}/lib)

file(GLOB_RECURSE SRC_FILES *.cpp)
file(GLOB_RECURSE HEADERS_FILES *.h *.hpp)
list(REMOVE_ITEM SRC_FILES ${ledger-core-jni-sources})
list(REMOVE_ITEM SRC_FILES dummy.cpp)
list(REMOVE_ITEM HEADERS_FILES ${ledger-core-jni-sources})

add_library(ledger-core-interface INTERFACE)

target_compile_definitions(ledger-core-interface INTERFACE ledger_core_EXPORTS)

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
	install(TARGETS ledger-core-static DESTINATION "lib")
endif()

if(UNIX AND NOT APPLE AND NOT ANDROID)
    target_link_libraries(ledger-core-interface INTERFACE -static-libstdc++)
endif()

target_link_libraries(ledger-core-interface INTERFACE bigd)

if(MSVC)
    add_definitions(-DHAVE_C99INCLUDES)
endif(MSVC)

if(MINGW OR MSVC)
    target_link_libraries(ledger-core-interface INTERFACE fmt-header-only)
else()
    target_link_libraries(ledger-core-interface INTERFACE fmt)
endif(MINGW OR MSVC)

target_link_libraries(ledger-core-interface INTERFACE crypto)
target_link_libraries(ledger-core-interface INTERFACE sqlite3)
target_link_libraries(ledger-core-interface INTERFACE soci_sqlite3)
target_link_libraries(ledger-core-interface INTERFACE soci_core_static)
target_link_libraries(ledger-core-interface INTERFACE leveldb)
target_link_libraries(ledger-core-interface INTERFACE spdlog)

#Secp256k1
target_link_libraries(ledger-core-interface INTERFACE "${CMAKE_SOURCE_DIR}/core/lib/secp256k1/lib/${CMAKE_STATIC_LIBRARY_PREFIX}secp256k1${CMAKE_STATIC_LIBRARY_SUFFIX}")
link_directories("${CMAKE_SOURCE_DIR}/core/lib/secp256k1/lib")
add_dependencies(ledger-core-interface INTERFACE secp256k1)


#Sqlite
target_link_libraries(ledger-core-interface INTERFACE $<TARGET_FILE:sqlite3>)
link_directories($<TARGET_FILE_DIR:sqlite3>)

file(GLOB_RECURSE LEDGER_CORE_HEADERS
        "*.hpp"
        "*.h"
        )

file(GLOB LEDGER_CORE_API_HEADERS
        "api/*.hpp"
        )

include_directories(ledger-core-interface INTERFACE ../lib/bigd)

if (TARGET_JNI)
    include_directories(ledger-core-interface INTERFACE ${JNI_INCLUDE_DIRS})
    target_link_libraries(ledger-core-interface INTERFACE ${JNI_LIBRARIES})
endif ()

file(GLOB ledger-core-header
        "*"
        )
file(COPY ${ledger-core-header} DESTINATION ${CMAKE_BINARY_DIR}/include/ledger/core FILES_MATCHING PATTERN "*.h")
file(COPY ${ledger-core-header} DESTINATION ${CMAKE_BINARY_DIR}/include/ledger/core FILES_MATCHING PATTERN "*.hpp")

target_include_directories(ledger-core INTERFACE .)
target_include_directories(ledger-core-interface INTERFACE ../lib/bigd)
target_include_directories(ledger-core-interface INTERFACE ../lib/fmt-3.0.0)
target_include_directories(ledger-core-interface INTERFACE ../lib/rapidjson/include)
target_include_directories(ledger-core-interface INTERFACE ../lib/soci/core)
target_include_directories(ledger-core-interface INTERFACE ../lib/soci_sqlite3)
target_include_directories(ledger-core-interface INTERFACE ../lib/sqlite3)
target_include_directories(ledger-core-interface INTERFACE ../lib/spdlog/include)
target_include_directories(ledger-core-interface INTERFACE ../lib/openssl/include)
target_include_directories(ledger-core-interface INTERFACE ../lib/leveldb/include)
target_include_directories(ledger-core-interface INTERFACE ../test/lib/boost)
target_include_directories(ledger-core-interface INTERFACE ../lib/cereal/)
target_include_directories(ledger-core-interface INTERFACE ../lib/date/)
target_include_directories(ledger-core-interface INTERFACE ../lib/secp256k1)

target_compile_definitions(ledger-core-interface INTERFACE LIB_VERSION_MAJOR=${VERSION_MAJOR} LIB_VERSION_MINOR=${VERSION_MINOR}
        LIB_VERSION_PATCH=${VERSION_PATCH})


        
install(TARGETS ledger-core DESTINATION "lib")
install(FILES ${LEDGER_CORE_API_HEADERS} DESTINATION "include/ledger-core")
