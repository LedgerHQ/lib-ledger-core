add_library (
        ledger-core SHARED
        bytes/BytesReader.cpp
        ledger-core.h
        math/BigInt.cpp utils/endian.cpp
        utils/endian.h
        utils/hex.cpp
        utils/json.hpp
        bytes/BytesWriter.cpp
        bytes/BytesWriter.h
        async/ExecutionContext.hpp
        async/Callback.hpp
        async/EventEmitter.hpp)
target_link_libraries(ledger-core bigd)
target_link_libraries(ledger-core fmt)

file(GLOB_RECURSE LEDGER_CORE_HEADERS
        "*.hpp"
        )
set_source_files_properties(${LEDGER_CORE_HEADERS} PROPERTIES LANGUAGE CXX)


include_directories(ledger-core ../lib/bigd)

file(GLOB ledger-core-header
        "*"
        )
file(COPY ${ledger-core-header} DESTINATION ${CMAKE_BINARY_DIR}/include/ledger/core FILES_MATCHING PATTERN "*.h")
file(COPY ${ledger-core-header} DESTINATION ${CMAKE_BINARY_DIR}/include/ledger/core FILES_MATCHING PATTERN "*.hpp")

target_include_directories (ledger-core PUBLIC ..)
target_include_directories (ledger-core PUBLIC ../lib/bigd)
target_include_directories (ledger-core PUBLIC ../lib/fmt-3.0.0)