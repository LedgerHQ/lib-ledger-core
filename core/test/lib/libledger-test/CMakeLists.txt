
add_library(ledger-test STATIC
        EventLooper.cpp EventLooper.hpp EventThread.cpp EventThread.hpp
        NativeThreadDispatcher.cpp NativeThreadDispatcher.hpp
        NativePathResolver.cpp NativePathResolver.hpp
        CoutLogPrinter.cpp CoutLogPrinter.hpp
        mongoose.c mongoose.h MongooseHttpClient.cpp MongooseHttpClient.hpp MongooseSimpleRestServer.cpp MongooseSimpleRestServer.hpp
        route.cc route.h OpenSSLRandomNumberGenerator.cpp OpenSSLRandomNumberGenerator.hpp PoolTestCaseBootstraper.cpp PoolTestCaseBootstraper.hpp callbacks.cpp callbacks.hpp AsioHttpClient.cpp AsioHttpClient.hpp)
include_directories(${CMAKE_BINARY_DIR}/include ../../../lib/openssl/include)
target_link_libraries(ledger-test PUBLIC crypto)
target_link_libraries(ledger-test PUBLIC ssl)
include_directories(../boost)
target_include_directories(ledger-test PUBLIC ../../../lib/fmt-3.0.0)
target_include_directories(ledger-test PUBLIC ../../../lib/cereal/)
target_include_directories(ledger-test PUBLIC ../asio/include/)

