include(ExternalProject)

#set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/leveldb")

include(ProjectSecp256k1)
add_subdirectory(bigd)
add_subdirectory(fmt-3.0.0)
add_subdirectory(sqlite3)
add_subdirectory(openssl)
add_subdirectory(snappy)

include(ProjectLeveldb)
add_subdirectory(leveldb)

add_subdirectory(spdlog)

# SOCI configuration
set(SOCI_STATIC ON)
set(SOCI_SHARED OFF)

add_subdirectory(soci)
add_subdirectory(soci_sqlite3)

#add_subdirectory(soci_mysql)