/*
 * Base64String
 *
 * Created by Rémi Barjon on 07/05/2020.
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

#include "B64String.hpp"

#include <math/BaseConverter.hpp>

namespace ledger {
namespace core {
namespace algorand {

    B64String::B64String(std::string b64)
            : b64(std::move(b64))
    {}

    const std::string& B64String::getRawString() const
    {
        return b64;
    }

    std::vector<uint8_t> B64String::toBinary() const
    {
        std::vector<uint8_t> binary;
        BaseConverter::decode(b64, BaseConverter::BASE64_RFC4648, binary);
        return binary;
    }

} // namespace ledger
} // namespace core
} // namespace algorand

