include(ExternalProject)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/leveldb")


include(ProjectSecp256k1)
add_subdirectory(bigd)
add_subdirectory(fmt)
find_package(OpenSSL)
if (NOT SYS_OPENSSL)
    add_subdirectory(openssl)
endif()
add_subdirectory(blake)
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

if (PG_SUPPORT)
    add_subdirectory(soci_postgresql)
endif()
