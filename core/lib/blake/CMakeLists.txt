project(blake)

# We just use the headers
add_library(blake INTERFACE)

target_include_directories(blake INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
