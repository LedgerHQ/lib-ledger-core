message(STATUS "=====================")
message(STATUS "COPYING CMAKELISTS")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "${CMAKE_SOURCE_DIR}")
message(STATUS "=====================")

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/cmake/leveldb/CMakeLists.txt
        DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/leveldb/)
