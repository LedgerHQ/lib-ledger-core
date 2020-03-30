
macro(set_project_settings version)

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'RelWithDebInfo' as none was specified.")

  set(CMAKE_BUILD_TYPE
      RelWithDebInfo
      CACHE STRING "Select build type" FORCE)

  # Set the possible values of build type for cmake-gui and cmake
  set_property(
    CACHE CMAKE_BUILD_TYPE
    PROPERTY STRINGS
             "Debug"
             "Release"
             "MinSizeRel"
             "RelWithDebInfo")
endif()

string(REPLACE "." ";" VERSION ${version})
list(GET VERSION 0 VERSION_MAJOR)
list(GET VERSION 1 VERSION_MINOR)
list(GET VERSION 2 VERSION_PATCH)

# the project version number.
set(VERSION_MAJOR ${VERSION_MAJOR} CACHE STRING "Project major version number.")
set(VERSION_MINOR ${VERSION_MINOR} CACHE STRING "Project minor version number.")
set(VERSION_PATCH ${VERSION_PATCH} CACHE STRING "Project patch version number.")
mark_as_advanced(VERSION_MAJOR VERSION_MINOR VERSION_PATCH)

# C++ version and standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_program(CCACHE ccache)
if(CCACHE)
  message(STATUS "Using ccache")
  set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
else()
  message(STATUS "ccache not found")
endif() 
endmacro()