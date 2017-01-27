/*
 *
 * P2PKHBitcoinLikeKeychain
 * ledger-core
 *
 * Created by Pierre Pollastri on 25/01/2017.
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
#include "P2PKHBitcoinLikeKeychain.hpp"

namespace ledger {
    namespace core {

        struct P2PKHKeychainPersistentState {

        };

        P2PKHBitcoinLikeKeychain::P2PKHBitcoinLikeKeychain(const std::shared_ptr<api::Configuration> &configuration,
                                                           const api::BitcoinLikeNetworkParameters &params,
                                                           int account,
                                                           const std::shared_ptr<api::BitcoinLikeExtendedPublicKey> &xpub,
                                                           const std::shared_ptr<Preferences> &preferences)
                : BitcoinLikeKeychain(configuration, params, account, xpub, preferences) {}

        bool P2PKHBitcoinLikeKeychain::markAsUsed(const std::string &address) {
            return false;
        }

        std::string P2PKHBitcoinLikeKeychain::getFreshAddress(BitcoinLikeKeychain::KeyPurpose purpose) const {
            return "";
        }

        bool P2PKHBitcoinLikeKeychain::isEmpty() const {
            return false;
        }

        std::vector<std::string> P2PKHBitcoinLikeKeychain::getAllObservableAddresses(off_t from, off_t to) const {
            return {};
        }

        std::vector<std::string>
        P2PKHBitcoinLikeKeychain::getFreshAddresses(BitcoinLikeKeychain::KeyPurpose purpose, size_t n) const {
            return {};
        }

        Option<BitcoinLikeKeychain::KeyPurpose>
        P2PKHBitcoinLikeKeychain::getAddressPurpose(const std::string &address) const {
            return Option<BitcoinLikeKeychain::KeyPurpose >();
        }

        Option<std::string> P2PKHBitcoinLikeKeychain::getAddressDerivationPath(const std::string &address) const {
            return Option<std::string>();
        }

        std::vector<std::string>
        P2PKHBitcoinLikeKeychain::getAllObservableAddresses(BitcoinLikeKeychain::KeyPurpose purpose, off_t from,
                                                            off_t to) const {
            return {};
        }
    }
}