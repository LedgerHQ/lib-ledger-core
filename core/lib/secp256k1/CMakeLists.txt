
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
#add_definitions(-DUSE_NUM_NONE)
#add_definitions(-DUSE_FIELD_10X26)
#add_definitions(-DUSE_SCALAR_8X32)
add_definitions(-DHAVE_CONFIG_H)
add_library(secp256k1 STATIC
        src/secp256k1.c
        src/gen_context.c
        )
target_include_directories(secp256k1 PUBLIC .)