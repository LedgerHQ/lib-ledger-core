include(CMakeFindDependencyMacro)
find_dependency(Qt5 COMPONENTS Core Widgets Network Gui Concurrent WebSockets)
find_dependency(Threads)

get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" DIRECTORY)
include(${SELF_DIR}/ledger-core.cmake)
