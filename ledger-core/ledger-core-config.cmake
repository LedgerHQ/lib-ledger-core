include(CMakeFindDependencyMacro)

if (BUILD_TESTS)
    find_dependency(Qt5 COMPONENTS Core Widgets Network Gui Concurrent WebSockets)
endif()
find_dependency(Threads)

if (SYS_OPENSSL)
    find_dependency(OpenSSL)
endif()

get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" DIRECTORY)
include(${SELF_DIR}/ledger-core.cmake)
