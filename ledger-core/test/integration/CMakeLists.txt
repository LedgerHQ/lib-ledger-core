cmake_minimum_required(VERSION 3.0)
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

file(GLOB_RECURSE TEST_FILES *.cpp)
add_library(ledger-core-integration-test
    ${TEST_FILES}
)

target_include_directories(
    ledger-core-integration-test PUBLIC
    # hack for centralized build
    # including parent directory allows header inclusions such as "<integration/...>"
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}> $<INSTALL_INTERFACE:include/ledger-test>
)

target_link_libraries(ledger-core-integration-test gtest gtest_main)

if(SYS_OPENSSL)
    find_package(OpenSSL REQUIRED COMPONENTS Crypto)
    target_link_libraries(ledger-core-integration-test OpenSSL::Crypto)
else()
    target_link_libraries(ledger-core-integration-test crypto)
endif()

target_link_libraries(ledger-core-integration-test ledger-core-bundle)
target_link_libraries(ledger-core-integration-test ledger-test)
target_link_libraries(ledger-core-integration-test ledger-qt-host)
