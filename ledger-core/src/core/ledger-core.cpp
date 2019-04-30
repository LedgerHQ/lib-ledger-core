/*
 *
 * ledger-core.cpp.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 16/05/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Ledger
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include "ledger-core.h"
#include <boost/preprocessor.hpp>

namespace ledger {
    namespace core {
        const int LIB_VERSION = LIB_VERSION_MAJOR << 16 | LIB_VERSION_MINOR << 8 | LIB_VERSION_PATCH;
        const int VERSION_MAJOR = LIB_VERSION_MAJOR;
        const int VERSION_MINOR = LIB_VERSION_MINOR;
        const int VERSION_PATCH = LIB_VERSION_PATCH;
        const std::string LIB_STRING_VERSION =
                        BOOST_PP_STRINGIZE(LIB_VERSION_MAJOR) "."
                        BOOST_PP_STRINGIZE(LIB_VERSION_MINOR) "."
                        BOOST_PP_STRINGIZE(LIB_VERSION_PATCH);
    }
}