include(ExternalProject)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if (SYS_SECP256K1)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(SECP256K1 REQUIRED IMPORTED_TARGET GLOBAL libsecp256k1)
  # Manually set the target's properties to use the static lib and tweak the include directory
  set_property(TARGET PkgConfig::SECP256K1 PROPERTY IMPORTED_CONFIGURATIONS Release)
  set_property(TARGET PkgConfig::SECP256K1 PROPERTY IMPORTED_LOCATION_RELEASE "${SECP256K1_STATIC_LIBRARIES}")
  # Have to add '/..'
  set_property(TARGET PkgConfig::SECP256K1 PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${SECP256K1_INCLUDEDIR}/..")
else()
  include(ProjectSecp256k1)
endif()
add_subdirectory(bigd)
add_subdirectory(fmt)
if (SYS_OPENSSL)
    find_package(OpenSSL)
else()
    add_subdirectory(openssl)
endif()
add_subdirectory(blake)
add_subdirectory(ethash)
add_subdirectory(snappy)
add_subdirectory(sha512256)

add_subdirectory(spdlog)

# SOCI configuration
set(SOCI_STATIC ON)
#set(SOCI_SHARED OFF)
set(SOCI_SHARED OFF CACHE BOOL "USE ONLY STATIC" FORCE)

add_subdirectory(soci)
# Fix undefined symbol related to dl library on Unix system
# The dl library is linked here to avoid to modify vendor cmake files
target_link_libraries(soci_core_static ${CMAKE_DL_LIBS})

if (PG_SUPPORT)
    add_subdirectory(soci_postgresql)
endif()
