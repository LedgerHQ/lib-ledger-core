cmake_minimum_required(VERSION 3.0)
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(${CMAKE_BINARY_DIR}/include)
include_directories(.)
include_directories(../asio/include/)
include_directories(../uri/include/)

include_directories()

link_directories(${CMAKE_BINARY_DIR}/core/test/lib/uri/src/)

add_library(ledger-asio-host 
AsioExecutionContext.cpp
AsioExecutionContextImpl.cpp
AsioHttpClient.cpp
RequestResponce.cpp
)



target_compile_definitions(ledger-asio-host PUBLIC ASIO_STANDALONE)
target_link_libraries(ledger-asio-host ledger-core)
target_link_libraries(ledger-asio-host network-uri)

include(CopyAndInstallImportedTargets)
copy_install_imported_targets(ledger-asio-host crypto ledger-core)