cmake_minimum_required(VERSION 3.0)
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)
include_directories(.)
include_directories(../lib/asio/include/)
if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

include_directories(../lib/libledger-test/)
include_directories(C:/Users/akorol/Programming/asio-core/core-asio/3rdparty/uri/include/)

link_directories(C:/Users/akorol/Programming/asio-core/core-asio/3rdparty/uri/src/)

add_executable(syncbanch 
blocksync_test.cpp
AsioExecutionContext.cpp
AsioHttpClient.cpp
RequestResponce.cpp
)



target_compile_definitions(syncbanch PUBLIC ASIO_STANDALONE)
target_link_libraries(syncbanch gtest gtest_main)
target_link_libraries(syncbanch ledger-core)
target_link_libraries(syncbanch network-uri.lib)
target_include_directories(syncbanch PUBLIC ../../../core/src)

include(CopyAndInstallImportedTargets)
copy_install_imported_targets(syncbanch crypto ledger-core)