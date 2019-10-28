
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(mangoose STATIC mongoose.c mongoose.h)

add_library(ledger-test STATIC
            EventLooper.cpp EventLooper.hpp EventThread.cpp EventThread.hpp
            NativeThreadDispatcher.cpp NativeThreadDispatcher.hpp
            NativePathResolver.cpp NativePathResolver.hpp
            CoutLogPrinter.cpp CoutLogPrinter.hpp
            MongooseHttpClient.cpp MongooseHttpClient.hpp MongooseSimpleRestServer.cpp MongooseSimpleRestServer.hpp
            route.cc route.h OpenSSLRandomNumberGenerator.cpp OpenSSLRandomNumberGenerator.hpp callbacks.cpp callbacks.hpp AsioHttpClient.cpp AsioHttpClient.hpp FakeWebSocketClient.cpp FakeWebSocketClient.h
            FakeHttpClient.hpp FakeHttpClient.cpp
            FakeUrlConnection.hpp FakeUrlConnection.cpp)
include_directories(${CMAKE_BINARY_DIR}/include ../../../lib/openssl/include)
if (MSVC)
    target_include_directories(ledger-test PUBLIC ${CMAKE_BINARY_DIR}/include/ledger/core)
endif ()

target_link_libraries(mangoose PUBLIC ssl)

target_link_libraries(ledger-test PUBLIC crypto)
target_link_libraries(ledger-test PUBLIC ledger-core-static)
target_link_libraries(ledger-test PUBLIC mangoose)
target_include_directories(ledger-test PUBLIC ../../../lib/fmt-3.0.0)
target_include_directories(ledger-test PUBLIC ../../../lib/cereal/)
target_include_directories(ledger-test PUBLIC ../asio/include/)
target_include_directories(ledger-test PUBLIC ../../../../core)
target_include_directories(ledger-test PUBLIC ../../../../core/src)
target_include_directories(ledger-test PUBLIC ../../../lib/boost/)