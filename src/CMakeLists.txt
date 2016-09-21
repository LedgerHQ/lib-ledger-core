# Create a library called "Hello" which includes the source file "hello.cxx".
# The extension is already found. Any number of sources could be listed here.

add_library(ledger-core SHARED bytes/BytesReader.cpp ../include/ledger-core.h math/BigInt.cpp utils/endian.cpp ../include/utils/endian.h utils/hex.cpp)
target_link_libraries(ledger-core bigd)

include_directories(ledger-core ../lib/bigd/)

target_include_directories (ledger-core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories (ledger-core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../lib/bigd)