/*
 *
 * TezosKey
 * ledger-core
 *
 * Created by Axel Haustant on 10/09/2020.
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
#include <algorithm>
#include <utils/Exception.hpp>
#include "TezosKey.h"

namespace ledger {
    namespace core {

        namespace TezosKeyType {
            std::ostream &operator<<(std::ostream &os, const Encoding &e) {
                return os << e.prefix;
            }

            std::vector<Encoding> ALL = {EDPK, SPPK, P2PK};
        }

        std::experimental::optional<TezosKeyType::Encoding>
        TezosKeyType::fromVersion(std::vector<uint8_t> version) {
            auto isVersion = [version](TezosKeyType::Encoding encoding) {
                return version == encoding.version;
            };
            auto encoding = std::find_if(TezosKeyType::ALL.begin(), TezosKeyType::ALL.end(), isVersion);

            if (encoding == TezosKeyType::ALL.end()) {
                // throw Exception(api::ErrorCode::INVALID_BASE58_FORMAT, "Base 58 invalid prefix");
                return {};
            }

            return {*encoding};
        }

        std::experimental::optional<TezosKeyType::Encoding>
        TezosKeyType::fromBase58(std::string key) {
            auto hasPrefix = [key](TezosKeyType::Encoding encoding) {
                return key.find(encoding.prefix, 0) == 0;
            };
            auto encoding = std::find_if(TezosKeyType::ALL.begin(), TezosKeyType::ALL.end(), hasPrefix);

            if (encoding == TezosKeyType::ALL.end()) {
                // throw Exception(api::ErrorCode::INVALID_BASE58_FORMAT, "Base 58 invalid prefix");
                return {};
            }

            return {*encoding};
        }
    }
}
