project(blake)

if(OPENSSL_PKG_VERSION STREQUAL "1.0")
    find_package(OpenSSL 1.0 EXACT REQUIRED COMPONENTS SSL Crypto)
elseif(OPENSSL_PKG_VERSION STREQUAL "1.1")
    find_package(OpenSSL 1.1 EXACT REQUIRED COMPONENTS SSL Crypto)
endif()

add_library(blake STATIC blake2b.c blake256.c)

target_link_libraries(blake PUBLIC ${CMAKE_DL_LIBS})
target_link_libraries(blake PUBLIC OpenSSL::Crypto)

target_include_directories(blake PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
