
add_library(ledger-test STATIC
        EventLooper.cpp EventLooper.hpp EventThread.cpp EventThread.hpp
        NativeThreadDispatcher.cpp NativeThreadDispatcher.hpp
        NativePathResolver.cpp NativePathResolver.hpp
        CoutLogPrinter.cpp CoutLogPrinter.hpp
        mongoose.c mongoose.h MongooseHttpClient.cpp MongooseHttpClient.hpp MongooseSimpleRestServer.cpp MongooseSimpleRestServer.hpp
        route.cc route.h OpenSSLRandomNumberGenerator.cpp OpenSSLRandomNumberGenerator.hpp)
include_directories(${CMAKE_BINARY_DIR}/include ../../../lib/openssl/include)
target_link_libraries(ledger-test PUBLIC crypto)
include_directories(../boost)