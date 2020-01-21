/*
 *
 * BitcoinLikeKeychainFactory
 * ledger-core
 *
 * Created by Pierre Pollastri on 14/06/2017.
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

#pragma once

#include <core/api/Currency.hpp>
#include <core/api/ExtendedKeyAccountCreationInfo.hpp>
#include <core/async/Future.hpp>
#include <core/collections/DynamicObject.hpp>

#include <bitcoin/keychains/BitcoinLikeKeychain.hpp>

namespace ledger {
    namespace core {
        class BitcoinLikeKeychainFactory {
        public:
            virtual std::shared_ptr<BitcoinLikeKeychain> build(int32_t index,
                                                         const DerivationPath &path,
                                                         const std::shared_ptr<DynamicObject>& configuration,
                                                         const api::ExtendedKeyAccountCreationInfo& info,
                                                         const std::shared_ptr<Preferences>& accountPreferences,
                                                         const api::Currency& currency
            ) = 0;
            virtual std::shared_ptr<BitcoinLikeKeychain> restore(int32_t index,
                                                           const DerivationPath &path,
                                                           const std::shared_ptr<DynamicObject>& configuration,
                                                           const std::string& databaseXpubEntry,
                                                           const std::shared_ptr<Preferences>& accountPreferences,
                                                           const api::Currency& currency
            ) = 0;
        };
    }
}
