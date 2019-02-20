/*
 *
 * P2WPKHBitcoinLikeKeychain
 *
 * Created by El Khalil Bellakrid on 19/02/2019.
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


#include "P2WPKHBitcoinLikeKeychain.hpp"

namespace ledger {
    namespace core {
        P2WPKHBitcoinLikeKeychain::P2WPKHBitcoinLikeKeychain(const std::shared_ptr<api::DynamicObject> &configuration,
                                                             const api::Currency &params,
                                                             int account,
                                                             const std::shared_ptr<api::BitcoinLikeExtendedPublicKey> &xpub,
                                                             const std::shared_ptr<Preferences> &preferences)
                : CommonBitcoinLikeKeychains(configuration, params, account, xpub, preferences)
        {
            _version = params.bitcoinLikeNetworkParameters.value().P2SHVersion;
            getAllObservableAddresses(0, _observableRange);
        }

        std::string P2SHBitcoinLikeKeychain::getAddressFromPubKey(const std::shared_ptr<api::BitcoinLikeExtendedPublicKey> &pubKey,
                                                                  const std::string& derivationPath) {
            auto config = std::make_shared<DynamicObject>();
            config->putString("keychainEngines", api::KeychainEngines::BIP49_P2SH);
            config->putData("version", _version);
            return BitcoinLikeAddress::fromPublicKey(pubKey, getCurrency(), derivationPath, config);
        }

        int32_t P2SHBitcoinLikeKeychain::getOutputSizeAsSignedTxInput() const {
            int32_t result = 0;
            //witness
            //1 byte for number of stack elements
            result += 1;
            //72 byte for signature (length + signature)
            result += 72;
            //40 byte of hash script (length + script)
            result += 40;
            return result;
        }
    }
}