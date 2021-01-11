
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(mangoose STATIC mongoose.c mongoose.h)

add_library(ledger-test STATIC
            EventLooper.cpp EventLooper.hpp EventThread.cpp EventThread.hpp
            NativeThreadDispatcher.cpp NativeThreadDispatcher.hpp
            NativePathResolver.cpp NativePathResolver.hpp
            CoutLogPrinter.cpp CoutLogPrinter.hpp
            MongooseHttpClient.cpp MongooseHttpClient.hpp MongooseSimpleRestServer.cpp MongooseSimpleRestServer.hpp
            route.cc route.h OpenSSLRandomNumberGenerator.cpp OpenSSLRandomNumberGenerator.hpp callbacks.cpp callbacks.hpp FakeWebSocketClient.cpp FakeWebSocketClient.h
            FakeHttpClient.hpp FakeHttpClient.cpp
            FakeUrlConnection.hpp FakeUrlConnection.cpp
            ExplorerStorage.hpp ExplorerStorage.cpp
            HttpClientOnFakeExplorer.hpp HttpClientOnFakeExplorer.cpp
            UvThreadDispatcher.hpp UvThreadDispatcher.cpp
            CppHttpLibClient.hpp CppHttpLibClient.cpp
            FilesystemUtils.cpp FilesystemUtils.hpp
            ProxyHttpClient.hpp ProxyHttpClient.cpp)
if (SYS_OPENSSL)
    include_directories(${CMAKE_BINARY_DIR}/include ${OPENSSL_INCLUDE_DIR})
else()
    include_directories(${CMAKE_BINARY_DIR}/include ../../../lib/openssl/include)
endif()
if (MSVC)
    target_include_directories(ledger-test PUBLIC ${CMAKE_BINARY_DIR}/include/ledger/core)
    target_compile_definitions(ledger-test PUBLIC WIN32_LEAN_AND_MEAN)
endif ()

target_link_libraries(ledger-test PUBLIC uv)

if (SYS_OPENSSL)
    find_package(OpenSSL)
    if(WIN32)
        target_link_libraries( mangoose PUBLIC crypt32 ws2_32)
        target_link_libraries( ledger-test PUBLIC crypt32 ws2_32)
    endif()
    target_link_libraries(mangoose PUBLIC OpenSSL::SSL)
    target_link_libraries(ledger-test PUBLIC OpenSSL::Crypto)
else()
    target_link_libraries(mangoose PUBLIC ssl)
    target_link_libraries(ledger-test PUBLIC crypto)
endif()

if(UNIX AND NOT APPLE AND NOT ANDROID)
    target_link_libraries(ledger-test PUBLIC stdc++fs)
endif()

target_link_libraries(ledger-test PUBLIC ledger-core-static)
target_link_libraries(ledger-test PUBLIC mangoose)
target_include_directories(ledger-test PUBLIC ../../../lib/fmt-3.0.0)
target_include_directories(ledger-test PUBLIC ../../../lib/cereal/)
target_include_directories(ledger-test PUBLIC ../../../../core)
target_include_directories(ledger-test PUBLIC ../../../../core/src)
target_include_directories(ledger-test PUBLIC ../../../lib/boost/)
target_include_directories(ledger-test PUBLIC ../libuv/include)
target_include_directories(ledger-test PUBLIC ../cpp-httplib/)
