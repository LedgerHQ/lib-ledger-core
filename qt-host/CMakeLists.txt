cmake_minimum_required(VERSION 2.8.11)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the QtWidgets library
find_package(Qt5 COMPONENTS Core Widgets Network Gui Concurrent)

# Tell CMake to create the helloworld executable
add_library(ledger-qt-host STATIC
            async/QtThreadDispatcher.cpp async/QtThreadDispatcher.hpp
            async/QtThreadPoolExecutionContext.cpp async/QtThreadPoolExecutionContext.hpp async/QtMainExecutionContext.cpp async/QtMainExecutionContext.hpp net/QtHttpClient.cpp net/QtHttpClient.hpp async/async_wait.h)

# Use the Widgets module from Qt 5.
target_link_libraries(ledger-qt-host Qt5::Core)
target_link_libraries(ledger-qt-host Qt5::Concurrent)
target_link_libraries(ledger-qt-host Qt5::Network)
target_link_libraries(ledger-qt-host ledger-core)