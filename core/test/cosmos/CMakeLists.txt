project(ledger-core-cosmos-tests)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(ledger-core 0.1.0 REQUIRED)

add_subdirectory(lib/googletest)

set(gtest_force_shared_crt ON CACHE BOOL "Build gtest with shared runtime" FORCE)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

add_executable(${PROJECT_NAME}
  main.cpp
  Fixtures.cpp
  address_test.cpp
  db_test.cpp
  transactions_test.cpp
  synchronization_test.cpp
  AccountTests.cpp
  )

target_link_libraries(${PROJECT_NAME} ledger-core-cosmos-static)
target_link_libraries(${PROJECT_NAME} Core::ledger-core-static)
target_link_libraries(${PROJECT_NAME} Core::ledger-test)
target_link_libraries(${PROJECT_NAME} Core::ledger-core-integration-test)
target_link_libraries(${PROJECT_NAME} Core::ledger-qt-host)
target_link_libraries(${PROJECT_NAME} gtest gtest_main)


include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})
