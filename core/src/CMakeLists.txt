# Create a library called "Hello" which includes the source file "hello.cxx".
# The extension is already found. Any number of sources could be listed here.

add_library (
        ledger-core SHARED
        bytes/BytesReader.cpp
        ledger-core.h
        math/BigInt.cpp utils/endian.cpp
        utils/endian.h
        utils/hex.cpp
        bytes/BytesWriter.cpp
        bytes/BytesWriter.h
        async/ExecutionContext.h
        async/Callback.hpp)
target_link_libraries(ledger-core bigd)
target_link_libraries(ledger-core fmt)

include_directories(ledger-core ../lib/bigd)

file(GLOB ledger-core-header
        "*"
        )
file(COPY ${ledger-core-header} DESTINATION ${CMAKE_BINARY_DIR}/include/ledger/core FILES_MATCHING PATTERN "*.h")
file(COPY ${ledger-core-header} DESTINATION ${CMAKE_BINARY_DIR}/include/ledger/core FILES_MATCHING PATTERN "*.hpp")

target_include_directories (ledger-core PUBLIC ..)
target_include_directories (ledger-core PUBLIC ../lib/bigd)
target_include_directories (ledger-core PUBLIC ../lib/fmt-3.0.0)