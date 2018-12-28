include(ExternalProject)

#set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/leveldb")

include(ProjectSecp256k1)
add_subdirectory(bigd)
add_subdirectory(fmt-3.0.0)
add_subdirectory(openssl)
add_subdirectory(ethash)
add_subdirectory(snappy)
add_subdirectory(leveldb)

add_subdirectory(spdlog)
if (SQLCIPHER)
    include(ProjectSQLiteCipher)
else()
    add_subdirectory(sqlite3)
endif()
# SOCI configuration
set(SOCI_STATIC ON)
#set(SOCI_SHARED OFF)
set(SOCI_SHARED OFF CACHE BOOL "USE ONLY STATIC" FORCE)

add_subdirectory(soci)
add_subdirectory(soci_sqlite3)

#add_subdirectory(soci_mysql)