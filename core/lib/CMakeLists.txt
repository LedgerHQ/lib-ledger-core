
add_subdirectory(bigd)
add_subdirectory(fmt-3.0.0)
add_subdirectory(sqlite3)
add_subdirectory(openssl)

# SOCI configuration

set(SOCI_STATIC ON)
set(SOCI_SHARED OFF)
set(SOCI_EMPTY OFF)
set(SQLITE3_LIBRARY $<TARGET_FILE:sqlite3>)
set(SQLITE3_INCLUDE_DIR ../sqlite3)
set(WITH_ODBC OFF)

add_subdirectory(soci)