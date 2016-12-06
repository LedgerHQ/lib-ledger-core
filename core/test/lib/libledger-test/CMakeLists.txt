
add_library(ledger-test STATIC
        EventLooper.cpp EventLooper.hpp EventThread.cpp EventThread.hpp
        NativeThreadDispatcher.cpp NativeThreadDispatcher.hpp
        NativePathResolver.cpp NativePathResolver.hpp
        CoutLogPrinter.cpp CoutLogPrinter.hpp
        mongoose.c mongoose.h MongooseHttpClient.cpp MongooseHttpClient.hpp MongooseSimpleRestServer.cpp MongooseSimpleRestServer.hpp
        route.cc route.h)
include_directories(${CMAKE_BINARY_DIR}/include)
include_directories(../boost)