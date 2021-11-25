option (USE_CODE_COVERAGE "Enable profiling and coverage report analysis" OFF)

if(USE_CODE_COVERAGE)
    if(CMAKE_COMPILER_IS_GNUCXX) # NOT WIN32
        list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/codecoverage")
        include(CodeCoverage)
        append_coverage_compiler_flags()
        setup_target_for_coverage_lcov(
            NAME coverage
            EXECUTABLE ctest --no-compress-output -T Test -L unit
            BASE_DIRECTORY "${PROJECT_SOURCE_DIR}/core/src"
            EXCLUDE 
                "/usr/*"
                "${PROJECT_BINARY_DIR}/*"
                "${PROJECT_SOURCE_DIR}/core/lib/*"
                "${PROJECT_SOURCE_DIR}/core/test/*"
        )
    endif()
endif()
