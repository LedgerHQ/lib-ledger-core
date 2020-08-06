/*
 *
 * StellarLikeKeychainFactory.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 21/02/2019.
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

#include "StellarLikeKeychainFactory.hpp"
#include "../keychains/DefaultStellarLikeKeychain.hpp"

namespace ledger {
    namespace core {

        std::shared_ptr<StellarLikeKeychain>
        StellarLikeKeychainFactory::build(int32_t index, const DerivationPath &path,
                                          const std::shared_ptr<DynamicObject> &configuration,
                                          const api::AccountCreationInfo &info,
                                          const std::shared_ptr<Preferences> &accountPreferences,
                                          const api::Currency &currency) {
            return std::make_shared<DefaultStellarLikeKeychain>(
                    std::make_shared<StellarLikeAddress>(info.publicKeys[0], currency, Option<uint64_t>::NONE, Option<std::string>(info.derivations[0])),
                    configuration,
                    currency, accountPreferences);
        }

        std::shared_ptr<StellarLikeKeychain>
        StellarLikeKeychainFactory::restore(int32_t index, const DerivationPath &path,
                                            const std::shared_ptr<DynamicObject> &configuration,
                                            const std::string &address,
                                            const std::shared_ptr<Preferences> &accountPreferences,
                                            const api::Currency &currency) {
            return std::make_shared<DefaultStellarLikeKeychain>(
                    std::make_shared<StellarLikeAddress>(address, currency, Option<std::string>("")),
                    configuration,
                    currency, accountPreferences);
        }
    }
}