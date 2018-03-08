# Sample toolchain file for building for Windows from an Ubuntu Linux system.
#
# Typical usage:
#    *) install cross compiler: `sudo apt-get install mingw-w64 g++-mingw-w64`
#    *) cd build
#    *) cmake -DCMAKE_TOOLCHAIN_FILE=~/Toolchain-Ubuntu-mingw64.cmake ..

set(CMAKE_SYSTEM_NAME Windows)
set(TOOLCHAIN_PREFIX vs-15-2017-win64)

#set(CMAKE_GENERATOR_PLATFORM "Visual Studio 15 2017 Win64")
set(CMAKE_GENERATOR "Visual Studio 15 2017 Win64")
set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)

set(POLLY_TOOLCHAIN_NAME "vs-15-2017-win64")
set(POLLY_TOOLCHAIN_TAG "vs-15")

include("${PROJECT_SOURCE_DIR}/toolchains/polly/vs-15-2017-win64.cmake")