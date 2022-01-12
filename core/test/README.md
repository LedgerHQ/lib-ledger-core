# Using HTTP cache in CI tests

By default CI tests do not actually request external web services. Instead pre-saved HTTP responses are used. An attempt to request an URL or an URL with parameters that do not exist in pre-saved cache will result in a runtime error. This default behavior can be modified with following CMake parameters:

TESTS\_UPDATE\_HTTP\_CACHE - to recreate or update files in http\_cache folders. Files are saved in CMake binary directory. It is necessary to copy updated files into the source tree (this doesn't happen automatically).

TESTS\_USE\_EXTERNAL\_WEB\_SERVICES - to run tests against real web services without pre-saved responses. This turns off HTTP request caching.

To switch between these options on the fly the corresponding value can be changed in ${CMAKE\_BINARY\_DIR}/CMakeCache.txt .
