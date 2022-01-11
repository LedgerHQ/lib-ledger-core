
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(ledger-test-proxy-http-client STATIC
            FakeUrlConnection.hpp FakeUrlConnection.cpp
            ProxyHttpClient.hpp ProxyHttpClient.cpp)

target_link_libraries(ledger-test-proxy-http-client PUBLIC ledger-core-static)
target_link_libraries(ledger-test-proxy-http-client PUBLIC gtest)

if (TESTS_UPDATE_HTTP_CACHE)
    message("ProxyHttpClient mode: update HTTP cache")
    set_property(SOURCE ProxyHttpClient.cpp PROPERTY COMPILE_DEFINITIONS UPDATE_HTTP_CACHE ALLOW_HTTP_ACCESS)
elseif (TESTS_RUN_WITHOUT_HTTP_CACHE)
    message("ProxyHttpClient mode: don't use HTTP cache")
    set_property(SOURCE ProxyHttpClient.cpp PROPERTY COMPILE_DEFINITIONS ALLOW_HTTP_ACCESS)
else ()
    message("ProxyHttpClient mode: use HTTP cache")
    set_property(SOURCE ProxyHttpClient.cpp PROPERTY COMPILE_DEFINITIONS LOAD_HTTP_CACHE)
endif()

