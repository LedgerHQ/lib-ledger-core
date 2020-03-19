/*
 *
 * url.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 13/01/2020.
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

#include "URL.h"
#include <sstream>
#include <iomanip>
#include <cctype>
#include "hex.h"

namespace ledger {
    namespace core {
        namespace url {
            std::string encodeUrlQuery(const std::string &urlQuery) {
                std::ostringstream out;
                out.fill('0');
                for (const auto& c : urlQuery) {
                    if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
                        out << c;
                    } else {
                        out << '%' << std::setw(2) << hex::toString({static_cast<uint8_t>(c)}, true);
                    }
                }
                return out.str();
            }
        }
    }
}
