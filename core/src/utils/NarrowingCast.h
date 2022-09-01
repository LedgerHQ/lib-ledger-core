/*
 *
 * NarrowingCast
 * ledger-core
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Ledger
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

#ifndef LEDGER_CORE_NARROWINGCAST_H
#define LEDGER_CORE_NARROWINGCAST_H

#include <api/ErrorCode.hpp>
#include <limits>
#include <string>
#include <utils/Exception.hpp>

namespace ledger {
    namespace core {

        template <typename To, typename From>
        inline To narrowing_cast(const From value) {
            static_assert(std::numeric_limits<To>::max() < std::numeric_limits<From>::max(),
                          "narrowing_cast used in non-narrowing context (max)");

            static_assert(std::numeric_limits<To>::min() > std::numeric_limits<From>::min(),
                          "narrowing_cast used in non-narrowing context (min)");

            if (value > static_cast<From>(std::numeric_limits<To>::max())) {
                throw make_exception(api::ErrorCode::OUT_OF_RANGE,
                                     "narrowing_cast: value {} is too big", value);
            }

            if (value < 0 && value < static_cast<From>(std::numeric_limits<To>::min())) {
                throw make_exception(api::ErrorCode::OUT_OF_RANGE,
                                     "narrowing_cast: value {} is too small", value);
            }

            return static_cast<To>(value);
        }

    } // namespace core
} // namespace ledger

#endif // LEDGER_CORE_NARROWINGCAST_H
