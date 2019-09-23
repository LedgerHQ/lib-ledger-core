/*
 *
 * soci
 * ledger-core
 *
 * Created by Pierre Pollastri on 02/06/2017.
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

#pragma once

#include <type-conversion-traits.h>

#include <core/utils/Option.hpp>

namespace soci {
    template <typename T>
    struct type_conversion<ledger::core::Option<T>> {
        typedef typename type_conversion<T>::base_type base_type;

        static void from_base(base_type const & in, indicator ind, ledger::core::Option<T> & out) {
            if (ind == i_null) {
                out = ledger::core::Option<T>();
            }
            else {
                T tmp = T();
                type_conversion<T>::from_base(in, ind, tmp);
                out = ledger::core::Option<T>(tmp);
            }
        }

        static void to_base(ledger::core::Option<T> const & in, base_type & out, indicator & ind) {
            if (!in.isEmpty()) {
                type_conversion<T>::to_base(in.getValue(), out, ind);
            }
            else {
                ind = i_null;
            }
        }

    };
}
