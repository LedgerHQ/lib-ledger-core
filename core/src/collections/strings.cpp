/*
 *
 * strings
 * ledger-core
 *
 * Created by Pierre Pollastri on 30/01/2017.
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
#include "strings.hpp"
#include <boost/algorithm/string.hpp>
#include "String.hpp"
#include "String.hpp"

namespace ledger {
    namespace core {
        namespace strings {

            bool startsWith(const std::string& str, const std::string& prefix) {
                return boost::starts_with(str, prefix);
            }

            std::function<std::string (const std::string&, const Option<std::string>&)> mkString(const std::string& separator) {
                return [separator] (const std::string& item, const Option<std::string>& carry) -> std::string {
                    if (carry.isEmpty()) {
                        return item;
                    } else {
                        return carry.getValue() + separator + item;
                    }
                };
            }

        }

        String operator "" _S(const char* str, size_t size) {
            return String(str, size);
        }

    }
}

std::ostream &operator<<(std::ostream & os, const ledger::core::String& str) {
    os << str.str();
    return os;
}