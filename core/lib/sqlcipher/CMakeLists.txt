project(sqlcipher)

add_library(sqlcipher STATIC sqlite3.c sqlite3.h sqlite3ext.h)

target_link_libraries(sqlcipher PUBLIC ${CMAKE_DL_LIBS})
target_link_libraries(sqlcipher PUBLIC crypto)
target_include_directories(sqlcipher PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
