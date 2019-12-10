cmake_minimum_required(VERSION 3.8)
project(leveldb C CXX)

option(INSTALL_HEADERS "Install header files" ON)

if(WIN32)
    add_definitions(
            -DWIN32
            -D_CRT_NONSTDC_NO_DEPRECATE
            -D_SCL_SECURE_NO_WARNINGS
            -D_CRT_SECURE_NO_WARNINGS
            -DNOMINMAX
            -DLEVELDB_ATOMIC_PRESENT
            -DLEVELDB_PLATFORM_WINDOWS
    )
endif(WIN32)

set(LEVELDB_SOURCES
        db/builder.cc
        db/c.cc
        db/dbformat.cc
        db/db_impl.cc
        db/db_iter.cc
        db/dumpfile.cc
        db/filename.cc
        db/log_reader.cc
        db/log_writer.cc
        db/memtable.cc
        db/repair.cc
        db/table_cache.cc
        db/version_edit.cc
        db/version_set.cc
        db/write_batch.cc
        helpers/memenv/memenv.cc
        port/port_posix_sse.cc
        table/block.cc
        table/block_builder.cc
        table/filter_block.cc
        table/format.cc
        table/iterator.cc
        table/merger.cc
        table/table.cc
        table/table_builder.cc
        table/two_level_iterator.cc
        util/arena.cc
        util/bloom.cc
        util/cache.cc
        util/coding.cc
        util/comparator.cc
        util/crc32c.cc
        util/env.cc
        util/env_posix.cc
        util/filter_policy.cc
        util/hash.cc
        util/histogram.cc
        util/logging.cc
        util/options.cc
        util/status.cc
    )
if(WIN32)
    set(LEVELDB_SOURCES ${LEVELDB_SOURCES}
            port/port_win.cc
            util/env_win.cc
            )
else()
    set(LEVELDB_SOURCES ${LEVELDB_SOURCES}
            port/port_posix.cc
            util/env_posix.cc
    )
endif(WIN32)

add_library(leveldb ${LEVELDB_SOURCES})


if(WIN32)
    add_definitions(-DLEVELDB_PLATFORM_WINDOWS -DOS_WIN -DWIN32_LEAN_AND_MEAN)
    if(MSVC)
        add_definitions(-DCOMPILER_MSVC)
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    add_definitions(-DLEVELDB_PLATFORM_POSIX -DOS_FREEBSD)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_definitions(-DLEVELDB_PLATFORM_POSIX -DOS_LINUX)
elseif(CYGWIN)
    add_definitions(-DLEVELDB_PLATFORM_POSIX -DOS_LINUX)
elseif(APPLE)
    add_definitions(-DLEVELDB_PLATFORM_POSIX -DOS_MACOSX)
elseif(ANDROID)
add_definitions(-DLEVELDB_PLATFORM_POSIX -DOS_ANDROID)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR} include)

if(WIN32)
    target_link_libraries(leveldb PUBLIC Iphlpapi.lib Shlwapi.lib)
endif(WIN32)

install(TARGETS leveldb
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        )

if(INSTALL_HEADERS)
    file(GLOB HEADERS include/leveldb/*.h)
    install(FILES ${HEADERS} DESTINATION include/leveldb)
    install(FILES helpers/memenv/memenv.h DESTINATION include)
endif()