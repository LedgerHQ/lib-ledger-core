# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(mongoose STATIC mongoose.c mongoose.h)

if(SYS_OPENSSL)
    find_package(OpenSSL)
    target_link_libraries(mongoose OpenSSL::SSL)
else()
    target_link_libraries(mongoose ssl)
endif()

add_library(ledger-test STATIC
    EventLooper.cpp EventLooper.hpp EventThread.cpp EventThread.hpp NativeThreadDispatcher.cpp
    NativeThreadDispatcher.hpp NativePathResolver.cpp NativePathResolver.hpp CoutLogPrinter.cpp
    CoutLogPrinter.hpp MongooseHttpClient.cpp MongooseHttpClient.hpp MongooseSimpleRestServer.cpp
    MongooseSimpleRestServer.hpp route.cc route.h OpenSSLRandomNumberGenerator.cpp
    OpenSSLRandomNumberGenerator.hpp callbacks.cpp callbacks.hpp AsioHttpClient.cpp
    AsioHttpClient.hpp FakeWebSocketClient.cpp FakeWebSocketClient.hpp FakeHttpClient.hpp
    FakeHttpClient.cpp FakeUrlConnection.hpp FakeUrlConnection.cpp
)

target_include_directories(
    ledger-test INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> $<INSTALL_INTERFACE:include/ledger-test>
)
target_include_directories(ledger-test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../asio/include)
target_link_libraries(ledger-test PUBLIC mongoose)
target_link_libraries(ledger-test PUBLIC ledger-core-static)
