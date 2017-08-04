
include(ExternalProject)

add_subdirectory(bigd)
add_subdirectory(fmt-3.0.0)
add_subdirectory(sqlite3)
add_subdirectory(openssl)
add_subdirectory(snappy)
add_subdirectory(leveldb)

# SOCI configuration

set(SOCI_STATIC ON)
set(SOCI_SHARED OFF)

add_subdirectory(soci)
add_subdirectory(soci_sqlite3)
