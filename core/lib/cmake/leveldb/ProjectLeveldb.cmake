#Copy CMakeLists in submodule
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/cmake/leveldb/CMakeLists.txt
        DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/leveldb/)
