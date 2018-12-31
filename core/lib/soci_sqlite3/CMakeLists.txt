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

if (SQLCIPHER)
    add_dependencies(soci_sqlite3 SQLCipher)
    target_link_libraries(soci_sqlite3 "${CMAKE_SOURCE_DIR}/core/lib/sqlcipher/lib/${CMAKE_STATIC_LIBRARY_PREFIX}sqlcipher${CMAKE_STATIC_LIBRARY_SUFFIX}")
    include_directories(soci_sqlite3 ../sqlcipher/include)
    target_include_directories(soci_sqlite3 PUBLIC ../sqlcipher/include)
    #For the moment we use prebuilt sqlcipher for arm64 and armv7
    #that are linked to Foundation and Security iOS Frameworks
    if (APPLE AND DEFINED CMAKE_OSX_ARCHITECTURES)
        string(FIND "${CMAKE_OSX_ARCHITECTURES}" "arm" IS_ARM)
        if(IS_ARM GREATER_EQUAL 0)
            find_library(FOUNDATION Foundation)
            target_link_libraries(soci_sqlite3 ${FOUNDATION})
            find_library(SECURITY Security)
            target_link_libraries(soci_sqlite3 ${SECURITY})
        endif()
    endif()
else()
    target_include_directories(soci_sqlite3 PUBLIC ../sqlite3 )
endif()
target_include_directories(soci_sqlite3 PUBLIC ../soci/core)
