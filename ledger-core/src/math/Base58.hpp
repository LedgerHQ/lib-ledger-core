/*
 *
 * Base58
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
#ifndef LEDGER_CORE_BASE58_HPP
#define LEDGER_CORE_BASE58_HPP

#include <vector>
#include <string>
#include "../utils/Try.hpp"
#include "../utils/Exception.hpp"
#include <api/DynamicObject.hpp>
#include <memory>

namespace ledger {
    namespace core {
        class Base58 {
        public:
            Base58() = delete;
            ~Base58() = delete;

            static std::string encode(const std::vector<uint8_t>& bytes, const std::shared_ptr<api::DynamicObject> &config);
            static std::string encodeWithChecksum(const std::vector<uint8_t>& bytes, const std::shared_ptr<api::DynamicObject> &config);
            static std::string encodeWithEIP55(const std::vector<uint8_t>& bytes);
            static std::string encodeWithEIP55(const std::string &address);

            static std::vector<uint8_t> decode(const std::string& str,
                                               const std::shared_ptr<api::DynamicObject> &config);
            static Try<std::vector<uint8_t>> checkAndDecode(const std::string& str,
                                                            const std::shared_ptr<api::DynamicObject> &config);


            static std::vector<uint8_t> computeChecksum(const std::vector<uint8_t>& bytes, const std::string &networkIdentifier = "");
        };


    }
}


#endif //LEDGER_CORE_BASE58_HPP
