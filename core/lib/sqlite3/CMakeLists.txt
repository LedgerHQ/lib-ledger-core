project(sqlite3)
add_library(sqlite3 STATIC sqlite3.c sqlite3.h sqlite3ext.h)
target_link_libraries(sqlite3 ${CMAKE_DL_LIBS})
target_include_directories(sqlite3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
