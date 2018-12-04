cmake_minimum_required(VERSION 3.0)
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)
if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

include_directories(../lib/libledger-test/)

file(GLOB ledger-core-fixtures-srcs
        "../fixtures/*.cpp"
        "../fixtures/*.h"
        )

add_executable(syncbanch blocksync_test.cpp)

target_link_libraries(syncbanch gtest gtest_main)
target_link_libraries(syncbanch ledger-core)
target_link_libraries(syncbanch ledger-test)
target_link_libraries(syncbanch ledger-qt-host)
target_include_directories(syncbanch PUBLIC ../../../core/src)
target_include_directories(syncbanch PUBLIC ../../../qt-host)

include(CopyAndInstallImportedTargets)
copy_install_imported_targets(syncbanch crypto ledger-core)