if (MSVC)
    add_library(
            boost_date_time STATIC
            src/gregorian/gregorian_types.cpp
            src/gregorian/greg_weekday.cpp
            src/gregorian/greg_month.cpp
            src/gregorian/date_generators.cpp
            src/posix_time/posix_time_types.cpp
    )
    target_include_directories(boost_date_time PUBLIC ../../../boost)
endif()