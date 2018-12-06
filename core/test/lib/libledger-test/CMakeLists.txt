
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(ledger-test STATIC
            EventLooper.cpp EventLooper.hpp EventThread.cpp EventThread.hpp
            NativeThreadDispatcher.cpp NativeThreadDispatcher.hpp
            NativePathResolver.cpp NativePathResolver.hpp
            CoutLogPrinter.cpp CoutLogPrinter.hpp
            mongoose.c mongoose.h MongooseHttpClient.cpp MongooseHttpClient.hpp MongooseSimpleRestServer.cpp MongooseSimpleRestServer.hpp
            route.cc route.h OpenSSLRandomNumberGenerator.cpp OpenSSLRandomNumberGenerator.hpp callbacks.cpp callbacks.hpp AsioHttpClient.cpp AsioHttpClient.hpp FakeWebSocketClient.cpp FakeWebSocketClient.h)
include_directories(${CMAKE_BINARY_DIR}/include ../../../lib/openssl/include)
if (MSVC)
    add_dependencies(ledger-test ledger-core)
    target_include_directories(ledger-test PUBLIC ${CMAKE_BINARY_DIR}/include/ledger/core ${CMAKE_BINARY_DIR}/include/ledger/core/export)
endif ()
target_link_libraries(ledger-test PUBLIC crypto)
target_link_libraries(ledger-test PUBLIC ssl)
include_directories(../boost)
target_include_directories(ledger-test PUBLIC ../../../lib/fmt-3.0.0)
target_include_directories(ledger-test PUBLIC ../../../lib/cereal/)
target_include_directories(ledger-test PUBLIC ../asio/include/)
target_include_directories(ledger-test PUBLIC ../../../../core)
target_include_directories(ledger-test PUBLIC ../../../../core/src)
