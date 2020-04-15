/*
 *
 * Bech32Factory
 *
 * Created by El Khalil Bellakrid on 18/02/2019.
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

#include "Bech32Factory.h"
#include <bitcoin/bech32/BTCBech32.h>
#include <bitcoin/bech32/BCHBech32.h>
#include <cosmos/bech32/CosmosBech32.hpp>
#include <api/CosmosBech32Type.hpp>
#include <utils/Exception.hpp>
namespace ledger {
    namespace core {
        Option<std::shared_ptr<Bech32>> Bech32Factory::newBech32Instance(const std::string &networkIdentifier) {
            const auto btcBech32Identifiers = std::vector<std::string>{"btc", "btc_testnet", "dgb", "ltc"};
            const auto cosmosBech32Identifiers = std::vector<std::string>{
            api::to_string(api::CosmosBech32Type::ADDRESS),
            api::to_string(api::CosmosBech32Type::ADDRESS_VAL),
            api::to_string(api::CosmosBech32Type::PUBLIC_KEY),
            api::to_string(api::CosmosBech32Type::PUBLIC_KEY_VAL)
        };
            if (std::find(btcBech32Identifiers.begin(), btcBech32Identifiers.end(), networkIdentifier) != btcBech32Identifiers.end()) {
                return Option<std::shared_ptr<Bech32>>(std::make_shared<BTCBech32>(networkIdentifier));
            } else if (networkIdentifier == "abc") {
                return Option<std::shared_ptr<Bech32>>(std::make_shared<BCHBech32>());
            } else if (std::find(cosmosBech32Identifiers.begin(), cosmosBech32Identifiers.end(), networkIdentifier) != cosmosBech32Identifiers.end()) {
                if (networkIdentifier == api::to_string(api::CosmosBech32Type::ADDRESS)) {
                    return Option<std::shared_ptr<Bech32>>(std::make_shared<CosmosBech32>(api::CosmosBech32Type::ADDRESS));
                }
                if (networkIdentifier == api::to_string(api::CosmosBech32Type::ADDRESS_VAL)) {
                    return Option<std::shared_ptr<Bech32>>(std::make_shared<CosmosBech32>(api::CosmosBech32Type::ADDRESS_VAL));
                }
                if (networkIdentifier == api::to_string(api::CosmosBech32Type::PUBLIC_KEY)) {
                    return Option<std::shared_ptr<Bech32>>(std::make_shared<CosmosBech32>(api::CosmosBech32Type::PUBLIC_KEY));
                }
                if (networkIdentifier == api::to_string(api::CosmosBech32Type::PUBLIC_KEY_VAL)) {
                    return Option<std::shared_ptr<Bech32>>(std::make_shared<CosmosBech32>(api::CosmosBech32Type::PUBLIC_KEY_VAL));
                }
                return Option<std::shared_ptr<Bech32>>();
            }
            return Option<std::shared_ptr<Bech32>>();
        }
    }
}
