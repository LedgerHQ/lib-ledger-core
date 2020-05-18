project(ledger-core-algorand-tests)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(ledger-core 0.1.0 REQUIRED)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

add_executable(ledger-core-algorand-tests
    main.cpp
    MsgpackHelpersTests.cpp
    AlgorandAddressTests.cpp
    AlgorandExplorerTests.cpp
)
target_link_libraries(ledger-core-algorand-tests Core::ledger-core-static)
target_link_libraries(ledger-core-algorand-tests Core::ledger-test)
target_link_libraries(ledger-core-algorand-tests Core::ledger-core-integration-test)
target_link_libraries(ledger-core-algorand-tests Core::ledger-qt-host)
target_link_libraries(ledger-core-algorand-tests ledger-core-algorand-static)
target_link_libraries(ledger-core-algorand-tests gtest gtest_main)

target_include_directories(ledger-core-algorand-tests PRIVATE ${CMAKE_SOURCE_DIR}/src/thirdparty/msgpack-c/include)

set(gtest_force_shared_crt ON CACHE BOOL "Build gtest with shared runtime" FORCE)

add_subdirectory(lib/googletest)


