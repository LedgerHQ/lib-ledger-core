/*
 *
 * Bech32Parameters
 *
 * Created by El Khalil Bellakrid on 14/02/2019.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ledger
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


#include "Bech32Parameters.h"
#include <api/ErrorCode.hpp>
#include <utils/Exception.hpp>
namespace ledger {
    namespace core {
        namespace Bech32Parameters {
            const Bech32Struct getBech32Params(const std::string &networkIdentifier) {
                if (networkIdentifier == "btc") {
                    static const Bech32Struct BITCOIN = {
                            "bc",
                            "1",
                            6,
                            {0x3b6a57b2ULL, 0x26508e6dULL, 0x1ea119faULL, 0x3d4233ddULL, 0x2a1462b3ULL},
                            {0x00},
                            {0x00}
                    };
                    return BITCOIN;
                } else if (networkIdentifier == "btc_testnet") {
                    static const Bech32Struct BITCOIN_TESTNET = {
                            "tb",
                            "1",
                            6,
                            {0x3b6a57b2ULL, 0x26508e6dULL, 0x1ea119faULL, 0x3d4233ddULL, 0x2a1462b3ULL},
                            {0x00},
                            {0x00}
                    };
                    return BITCOIN_TESTNET;
                } else if (networkIdentifier == "abc") {
                    static const Bech32Struct BITCOIN_CASH = {
                            "bitcoincash",
                            ":",
                            8,
                            {0x98f2bc8e61ULL, 0x79b76d99e2ULL, 0xf33e5fb3c4ULL, 0xae2eabe2a8ULL, 0x1e4f43e470ULL},
                            {0x00},
                            {0x08}
                    };
                    return BITCOIN_CASH;
                }
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "No Bech32 parameters set for {}", networkIdentifier);
            }
        }
    }
}