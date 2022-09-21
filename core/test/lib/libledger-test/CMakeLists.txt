
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(mangoose STATIC mongoose.c mongoose.h)

add_library(ledger-test STATIC
            CoutLogPrinter.cpp CoutLogPrinter.hpp
            CppHttpLibClient.hpp CppHttpLibClient.cpp
            EventLooper.cpp EventLooper.hpp
            EventThread.cpp EventThread.hpp
            ExplorerStorage.hpp ExplorerStorage.cpp
            FakeHttpClient.hpp FakeHttpClient.cpp
            FakeWebSocketClient.cpp FakeWebSocketClient.h
            FilesystemUtils.cpp FilesystemUtils.hpp
            HttpClientOnFakeExplorer.hpp HttpClientOnFakeExplorer.cpp
            MemPreferencesBackend.hpp MemPreferencesBackend.cpp
            MongooseHttpClient.cpp MongooseHttpClient.hpp
            MongooseSimpleRestServer.cpp MongooseSimpleRestServer.hpp
            NativePathResolver.cpp NativePathResolver.hpp
            NativeThreadDispatcher.cpp NativeThreadDispatcher.hpp
            OpenSSLRandomNumberGenerator.cpp OpenSSLRandomNumberGenerator.hpp
            UvThreadDispatcher.hpp UvThreadDispatcher.cpp
            callbacks.cpp callbacks.hpp
            route.cc route.h ProxyCoreTracer.cpp ProxyCoreTracer.h)

if (SYS_OPENSSL)
    include_directories(${CMAKE_BINARY_DIR}/include ${OPENSSL_INCLUDE_DIR})
else()
    include_directories(${CMAKE_BINARY_DIR}/include ../../../lib/openssl/include)
endif()
if (MSVC)
    target_include_directories(ledger-test PUBLIC ${CMAKE_BINARY_DIR}/include/ledger/core)
    target_compile_definitions(ledger-test PUBLIC WIN32_LEAN_AND_MEAN)
endif ()

if (SYS_LIBUV)
  target_link_libraries(ledger-test PUBLIC PkgConfig::UV)
else()
  target_link_libraries(ledger-test PUBLIC uv)
endif()

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

if(UNIX AND NOT ANDROID)
    target_link_libraries(ledger-test PUBLIC $<$<CXX_COMPILER_ID:GNU>: stdc++fs> )
endif()

add_subdirectory(proxy-http-client)

target_link_libraries(ledger-test PUBLIC ledger-test-proxy-http-client)
target_link_libraries(ledger-test PUBLIC ledger-core-static)
target_link_libraries(ledger-test PUBLIC mangoose)
target_include_directories(ledger-test PUBLIC ../../../lib/fmt-3.0.0)
target_include_directories(ledger-test PUBLIC ../../../lib/cereal/)
target_include_directories(ledger-test PUBLIC ../../../../core)
target_include_directories(ledger-test PUBLIC ../../../../core/src)
target_include_directories(ledger-test PUBLIC ../../../lib/boost/)
target_include_directories(ledger-test PUBLIC ../libuv/include)
target_include_directories(ledger-test PUBLIC ../cpp-httplib/)

