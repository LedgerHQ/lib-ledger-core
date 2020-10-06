cmake_minimum_required(VERSION 2.8.11)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Tell CMake to create the helloworld executable
add_library(ledger-qt-host STATIC utils/FilesystemUtils.cpp utils/FilesystemUtils.hpp)

target_link_libraries(ledger-qt-host ledger-core-static)
target_include_directories(ledger-qt-host PUBLIC ../core/src)

if(UNIX AND NOT APPLE AND NOT ANDROID)
    target_link_libraries(ledger-qt-host PUBLIC stdc++fs)
endif()