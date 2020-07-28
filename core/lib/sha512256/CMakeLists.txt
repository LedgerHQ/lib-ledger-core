project(sha512256)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(sha512256 STATIC sha512_256.cpp)

target_compile_definitions(sha512256 PRIVATE "__STDC_WANT_LIB_EXT1__=1")

target_include_directories(sha512256 PUBLIC ../boost)
