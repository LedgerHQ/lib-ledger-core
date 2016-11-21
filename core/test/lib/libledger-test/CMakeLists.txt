
add_library(ledger-test STATIC EventLooper.cpp EventLooper.hpp EventThread.cpp EventThread.hpp)
include_directories(${CMAKE_BINARY_DIR}/include)