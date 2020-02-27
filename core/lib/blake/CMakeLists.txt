project(blake)

add_library(blake STATIC blake2b.cpp blake2b.h blake256.c blake256.h)

target_link_libraries(blake PUBLIC ${CMAKE_DL_LIBS})
target_link_libraries(blake PUBLIC OpenSSL::Crypto)

target_include_directories(blake PUBLIC ${OPENSSL_INCLUDE_DIR})
target_include_directories(blake PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
