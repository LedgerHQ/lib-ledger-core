include(ExternalProject)

#set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/leveldb")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}/core/lib/protobuf/cmake/cmake")


include(ProjectSecp256k1)
add_subdirectory(bigd)
add_subdirectory(fmt)
add_subdirectory(openssl)
add_subdirectory(ethash)
add_subdirectory(snappy)
add_subdirectory(leveldb)

add_subdirectory(spdlog)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Android")
  # On Android, temporary directories are a bit special and are not located in the same place, such
  # as /tmp or /usr/tmp; for this reason, we ask temporary files to be stored in memory instead
  message("Compiling SQLCipher for Android with file temporary directory disabled (all-on-memory).")
  add_definitions(-DSQLITE_TEMP_STORE=3)
endif ()

add_subdirectory(sqlcipher)

set(SQLITE_LIB sqlcipher)

# SOCI configuration
set(SOCI_STATIC ON)
#set(SOCI_SHARED OFF)
set(SOCI_SHARED OFF CACHE BOOL "USE ONLY STATIC" FORCE)

add_subdirectory(soci)
add_subdirectory(soci_sqlite3)

set(protobuf_BUILD_TESTS OFF CACHE BOOL "No test for protobuf")
set(protobuf_BUILD_PROTOC_BINARIES OFF CACHE BOOL "Protoc is installed separatly, to simplify cross-compilation")
set(protobuf_WITH_ZLIB OFF CACHE BOOL "Do not compress messages")
set(protobuf_MSVC_STATIC_RUNTIME OFF CACHE BOOL "Dynamic runtime for MSVC" FORCE)
add_subdirectory(protobuf/cmake)

set(ubinder_BUILDING_WRAPPER ON CACHE BOOL "Don't build ubinder, just include headers" FORCE)
add_subdirectory(ubinder)

#add_subdirectory(soci_mysql)