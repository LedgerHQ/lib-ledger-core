/*
 *
 * RippleLikeKeychainFactory
 *
 * Created by El Khalil Bellakrid on 06/01/2019.
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


#ifndef LEDGER_CORE_RIPPLELIKEKEYCHAINFACTORY_H
#define LEDGER_CORE_RIPPLELIKEKEYCHAINFACTORY_H


#include <collections/DynamicObject.hpp>

#include <api/ExtendedKeyAccountCreationInfo.hpp>
#include <api/Currency.hpp>

#include <wallet/ripple/keychains/RippleLikeKeychain.h>
#include <preferences/Preferences.hpp>

namespace ledger {
    namespace core {
        class RippleLikeKeychainFactory {
        public:
            std::shared_ptr<RippleLikeKeychain> build(int32_t index,
                                                        const DerivationPath &path,
                                                        const std::shared_ptr<DynamicObject>& configuration,
                                                        const api::ExtendedKeyAccountCreationInfo& info,
                                                        const std::shared_ptr<Preferences>& accountPreferences,
                                                        const api::Currency& currency);
            std::shared_ptr<RippleLikeKeychain> restore(int32_t index,
                                                          const DerivationPath &path,
                                                          const std::shared_ptr<DynamicObject>& configuration,
                                                          const std::string &databaseXpubEntry,
                                                          const std::shared_ptr<Preferences>& accountPreferences,
                                                          const api::Currency& currency);
        };
    }
}


#endif //LEDGER_CORE_RIPPLELIKEKEYCHAINFACTORY_H
