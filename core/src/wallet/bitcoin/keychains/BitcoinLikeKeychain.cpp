/*
 *
 * BitcoinLikeKeychain
 * ledger-core
 *
 * Created by Pierre Pollastri on 17/01/2017.
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

#include "BitcoinLikeKeychain.hpp"

namespace ledger {
    namespace core {

        BitcoinLikeKeychain::BitcoinLikeKeychain(
                const std::shared_ptr<api::DynamicObject> configuration,
                const api::BitcoinLikeNetworkParameters &params, int account,
                const std::shared_ptr<api::BitcoinLikeExtendedPublicKey> &xpub,
                const std::shared_ptr<Preferences> preferences) :
            _params(params), _account(account), _xpub(xpub), _preferences(preferences), _configuration(configuration) {

        }

        int BitcoinLikeKeychain::getAccountIndex() const {
            return _account;
        }

        const api::BitcoinLikeNetworkParameters &BitcoinLikeKeychain::getNetworkParameters() const {
            return _params;
        }

        std::shared_ptr<api::BitcoinLikeExtendedPublicKey> BitcoinLikeKeychain::getExtendedPublicKey() const {
            return _xpub;
        }

        bool BitcoinLikeKeychain::markAsUsed(const std::vector<std::string> &addresses) {
            bool result = false;
            for (auto& address : addresses) {
                result = markAsUsed(address) || result;
            }
            return result;
        }

        std::shared_ptr<Preferences> BitcoinLikeKeychain::getPreferences() const {
            return _preferences;
        }

        std::shared_ptr<api::DynamicObject> BitcoinLikeKeychain::getConfiguration() const {
            return _configuration;
        }
    }
}