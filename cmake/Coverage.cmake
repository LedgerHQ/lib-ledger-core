option (CODE_COVERAGE "Enable profiling and coverage report analysis" OFF)

if(CODE_COVERAGE)
    if(CMAKE_COMPILER_IS_GNUCXX) # NOT WIN32
        list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/codecoverage")
        include(CodeCoverage)
        append_coverage_compiler_flags()

        set(COVERAGE_EXCLUDES
            "/usr/*"
            "${PROJECT_BINARY_DIR}/*"
            "${PROJECT_SOURCE_DIR}/core/lib/*"
            "${PROJECT_SOURCE_DIR}/core/test/*"
        )

        setup_target_for_coverage_lcov(
            NAME coverage_unit
            EXECUTABLE ctest -T Test -L unit
            BASE_DIRECTORY "${PROJECT_SOURCE_DIR}/core/src"
        )
        setup_target_for_coverage_lcov(
            NAME coverage_all
            EXECUTABLE ctest -T Test
            BASE_DIRECTORY "${PROJECT_SOURCE_DIR}/core/src"
        )

    endif()
endif()
