add_subdirectory(lib)
add_subdirectory(src)

string(FIND CMAKE_OSX_SYSROOT "iphone" IS_IOS)
if (NOT IS_IOS)
    enable_testing()
    add_subdirectory(test)
endif ()
