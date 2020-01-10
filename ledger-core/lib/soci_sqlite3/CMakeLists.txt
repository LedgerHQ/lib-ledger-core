###############################################################################
#
# This file is part of CMake configuration for SOCI library
#
# Copyright (C) 2010 Mateusz Loskot <mateusz@loskot.net>
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#
###############################################################################

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_library(soci_sqlite3 STATIC
        blob.cpp common.cpp common.h error.cpp factory.cpp row-id.cpp session.cpp soci-sqlite3.h standard-into-type.cpp
        standard-use-type.cpp statement.cpp vector-into-type.cpp vector-use-type.cpp)

target_link_libraries(soci_sqlite3 PUBLIC ${SQLITE_LIB})
target_include_directories(
    soci_sqlite3 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../soci/core>
    $<INSTALL_INTERFACE:include/soci>
)
