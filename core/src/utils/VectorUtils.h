/*
 *
 * VectorUtils
 * ledger-core
 *
 * Created by Pierre Pollastri on 03/08/2017.
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
#ifndef LEDGER_CORE_VECTORUTILS_H
#define LEDGER_CORE_VECTORUTILS_H

#include <vector>

namespace ledger {
    namespace core {
        class VectorUtils {
        public:
            template <typename T>
            static void padOnLeft(std::vector<T>& vector, T elem, std::size_t length) {
                if (vector.size() < length) {
                    auto maxPadAddress = length - vector.size();
                    std::vector<T> result(length);
                    auto j = 0;
                    for (auto i = 0; i < length; i++) {
                        if (i < maxPadAddress) {
                            result[i] = elem;
                        } else {
                            result[i] = vector[j];
                            j += 1;
                        }
                    }
                    result.swap(vector);
                }
            }
        };
    }
}


#endif //LEDGER_CORE_VECTORUTILS_H
