/*
 *
 * RippleLikeUtils.hpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 18/05/2020.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Ledger
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

#ifndef LEDGER_CORE_RIPPLELIKEUTILS_HPP
#define LEDGER_CORE_RIPPLELIKEUTILS_HPP

#include <chrono>

namespace ledger {
    namespace core {
        class RippleLikeUtils {
        public:
            RippleLikeUtils() = delete;
            ~RippleLikeUtils() = delete;

            /**
             * Convert a XRP timestamp to a C++ time point. Rippled is using another epoch time than UNIX
             * therefore it needs to be translated to UNIX timestamp before converting it to time point.
             * https://xrpl.org/basic-data-types.html#specifying-time
             * @param timestamp A XRP timestamp
             * @param time Reference to the time point to inflate.
             * @return The translated time point
             */
            static void xrpTimestampToTimePoint(uint64_t timestamp, std::chrono::system_clock::time_point &time);
        };
    }
}


#endif //LEDGER_CORE_RIPPLELIKEUTILS_HPP
