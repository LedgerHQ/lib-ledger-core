/*
 *
 * erc20Tokens
 *
 * Created by El Khalil Bellakrid on 26/08/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ledger
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


#include "erc20Tokens.h"
#include <api/ErrorCode.hpp>
#include <utils/Exception.hpp>
namespace ledger {
    namespace core {
        namespace erc20Tokens {
            const api::ERC20Token getERC20Token(const std::string &erc20) {
                if (erc20 == "ledger_coin") {
                    static const api::ERC20Token LEDGER_COIN("Ledger Coin",
                                                             "LGC",
                                                             "0x9549e8a940062615cee20c0420c98c25ffa2b214",
                                                             2);
                    return LEDGER_COIN;
                }
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "No known ERC20 token named {}", erc20);
            }

            const std::map<std::string, std::string> ERC20MethodsID({
                    std::pair<std::string, std::string>("totalSupply", "18160ddd"),
                    std::pair<std::string, std::string>("balanceOf", "70a08231"),
                    std::pair<std::string, std::string>("transfer", "a9059cbb"),
                    std::pair<std::string, std::string>("transferFrom", "23b872dd"),
                    std::pair<std::string, std::string>("approve", "095ea7b3")
                                                                    });

            const std::map<std::string, api::ERC20Token> ALL_ERC20({
                                                                           std::pair<std::string, api::ERC20Token>("0x9549e8a940062615cee20c0420c98c25ffa2b214", getERC20Token("ledger_coin"))
                                                                   });
        }
    }
}
