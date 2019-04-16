# Based on original work by David Manura
# Copyright (C) 2007-2012 LuaDist.
# Copyright (C) 2013 Brian Sidebotham

# Redistribution and use of this file is allowed according to the terms of the
# MIT license.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

if( BUILD_SHARED_LIBS )
    add_definitions ( -DOPENSSL_BUILD_SHLIBSSL )
endif()

FILE(GLOB LIBSRC *.c *.h)
list(REMOVE_ITEM LIBSRC
        ${CMAKE_CURRENT_SOURCE_DIR}/ssl_task.c
        ${CMAKE_CURRENT_SOURCE_DIR}/ssltest.c
        ${CMAKE_CURRENT_SOURCE_DIR}/clienthellotest.c
        ${CMAKE_CURRENT_SOURCE_DIR}/heartbeat_test.c
        )
if(BUILD_OBJECT_LIBRARY_ONLY)
    set( LIBHDR ssl.h kssl.h ssl2.h ssl3.h tls1.h dtls1.h ssl23.h srtp.h)
    file( COPY ${LIBHDR} DESTINATION openssl )
    add_library( ssl OBJECT ${LIBSRC} ${LIBHDR} )
else()
    add_library( ssl ${LIBSRC} )
    target_link_libraries( ssl crypto )

    install( FILES ssl.h ssl2.h ssl3.h ssl23.h tls1.h dtls1.h kssl.h
            DESTINATION include/openssl )

    install( TARGETS ssl
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib )
endif()

target_compile_definitions(ssl PRIVATE
        -DOPENSSL_NO_KRB5
        -DOPENSSL_NO_GMP
        -DOPENSSL_NO_JPAKE
        -DOPENSSL_NO_MD2
        -DOPENSSL_NO_RC5
        -DOPENSSL_NO_RFC3779
        -DOPENSSL_NO_STORE
        -DOPENSSL_NO_DYNAMIC_ENGINE
        -DOPENSSL_NO_SCTP
        -DOPENSSL_NO_EC_NISTP_64_GCC_128
        )
target_include_directories(ssl PUBLIC ../include)
target_include_directories(ssl PUBLIC ../include/openssl)