add_subdirectory(lib)
add_subdirectory(src)

string(FIND CMAKE_OSX_SYSROOT "iphone" IS_IOS)
if (IS_IOS LESS 0)
    enable_testing()
    add_subdirectory(test)
endif ()
