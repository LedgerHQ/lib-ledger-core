project(sqlcipher)

add_library(sqlcipher STATIC sqlite3.c sqlite3.h sqlite3ext.h)

target_link_libraries(sqlcipher PUBLIC ${CMAKE_DL_LIBS})

if (SYS_OPENSSL)
    target_link_libraries(sqlcipher PUBLIC OpenSSL::Crypto)
else()
    target_link_libraries(sqlcipher PUBLIC crypto)
endif()

target_include_directories(
    sqlcipher PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)
