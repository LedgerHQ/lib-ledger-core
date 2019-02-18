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
#include "BTCBech32.h"
#include "BCHBech32.h"
#include <utils/Exception.hpp>
namespace ledger {
    namespace core {
        std::shared_ptr<Bech32> Bech32Factory::newBech32Instance(const std::string &networkIdentifier) {
            if (networkIdentifier.compare("btc") == 0) {
                return std::make_shared<BTCBech32>();
            } else if (networkIdentifier.compare("abc") == 0) {
                return std::make_shared<BCHBech32>();
            }
            throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "Can not instanciate Bech32 for {}", networkIdentifier);
        }
    }
}