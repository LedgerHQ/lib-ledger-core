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
#include <utils/hex.h>
#include <collections/strings.hpp>
#include <math/BigInt.h>
using namespace soci;
namespace ledger {
    namespace core {
        namespace Bech32Parameters {
            const Bech32Struct getBech32Params(const std::string &networkIdentifier) {
                if (networkIdentifier == "btc") {
                    static const Bech32Struct BITCOIN = {
                            "bitcoin",
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
                            "bitcoin_testnet",
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
                            "bitcoin_cash",
                            "bitcoincash",
                            ":",
                            8,
                            {0x98f2bc8e61ULL, 0x79b76d99e2ULL, 0xf33e5fb3c4ULL, 0xae2eabe2a8ULL, 0x1e4f43e470ULL},
                            {0x00},
                            {0x08}
                    };
                    return BITCOIN_CASH;
                } else if (networkIdentifier == "dgb") {
                    //https://github.com/digibyte/digibyte/blob/master/src/bech32.cpp
                    static const Bech32Struct DIGIBYTE = {
                            "digibyte",
                            "dgb1",
                            "1",
                            6,
                            {0x3b6a57b2ULL, 0x26508e6dULL, 0x1ea119faULL, 0x3d4233ddULL, 0x2a1462b3ULL},
                            {0x00},
                            {0x00}
                    };
                    return DIGIBYTE;
                }
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "No Bech32 parameters set for {}", networkIdentifier);
            }

            const std::vector<Bech32Struct> ALL(
                    {
                            getBech32Params("btc"),
                            getBech32Params("btc_testnet"),
                            getBech32Params("abc"),
                            getBech32Params("dgb")
                    }
            );

            bool insertParameters(soci::session& sql, const Bech32Struct &params) {
                auto count = 0;
                sql << "SELECT COUNT(*) FROM bech32_parameters WHERE name = :name",
                        soci::use(params.name),
                        soci::into(count);
                if (count == 0) {
                    std::stringstream generator;
                    std::vector<std::string> strGenerator;
                    std::string separator(",");
                    for (auto& g : params.generator) {
                        BigInt bigIntG(g);
                        strGenerator.push_back(bigIntG.toString());
                    }
                    strings::join(strGenerator, generator, separator);
                    auto P2WPKHVersion = hex::toString(params.P2WPKHVersion);
                    auto P2WSHVersion = hex::toString(params.P2WSHVersion);
                    auto generatorStr = generator.str();
                    sql << "INSERT INTO bech32_parameters VALUES(:name, :hrp, :separator, :generator, :p2wpkh_version, :p2wsh_version)",
                            use(params.name),
                            use(params.hrp),
                            use(params.separator),
                            use(generatorStr),
                            use(P2WPKHVersion),
                            use(P2WSHVersion);
                    return true;
                }
                return false;
            }
        }
    }
}