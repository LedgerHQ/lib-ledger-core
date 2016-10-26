add_library (
        ledger-device SHARED
        IDevice.hpp IDeviceFactory.hpp)

file(GLOB_RECURSE LEDGER_CORE_HEADERS
        "*.hpp"
        )

target_link_libraries(ledger-device ledger-core)

include_directories(ledger-device ${CMAKE_BINARY_DIR}/include)

file(GLOB ledger-device-header
        "*"
        )
file(COPY ${ledger-device-header} DESTINATION ${CMAKE_BINARY_DIR}/include/ledger/device FILES_MATCHING PATTERN "*.h")
file(COPY ${ledger-core-header} DESTINATION ${CMAKE_BINARY_DIR}/include/ledger/device FILES_MATCHING PATTERN "*.hpp")

target_include_directories (ledger-device PUBLIC ..)
