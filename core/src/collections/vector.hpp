/*
 *
 * vector
 * ledger-core
 *
 * Created by Pierre Pollastri on 12/12/2016.
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
#ifndef LEDGER_CORE_VECTOR_HPP
#define LEDGER_CORE_VECTOR_HPP

#include <vector>
#include <functional>

namespace ledger {
    namespace core {
        namespace vector {

            template <typename T>
            std::vector<T> concat(const std::vector<T>& a,  const std::vector<T>& b) {
                std::vector<T> result;
                result.reserve(a.size() + b.size());
                result.insert(result.end(), a.begin(), a.end());
                result.insert(result.end(), b.begin(), b.end());
                return result;
            }

            template <typename U, typename T>
            inline std::vector<U> map(const std::vector<T>& source,
                     const std::function<U (const T&)>& f) {
                std::vector<U> out;
                for (const auto& item : source) {
                    out.push_back(f(item));
                }
                return out;
            };
        };
    }
}


#endif //LEDGER_CORE_VECTOR_HPP
