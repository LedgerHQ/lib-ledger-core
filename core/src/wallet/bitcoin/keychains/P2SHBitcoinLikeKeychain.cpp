/*
 *
 * P2SHBitcoinLikeKeychain
 * ledger-core
 *
 * Created by El Khalil Bellakrid on 30/04/2018.
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
#include "P2SHBitcoinLikeKeychain.hpp"
#include "../../../crypto/HASH160.hpp"
#include <bitcoin/BitcoinLikeAddress.hpp>

namespace ledger {
    namespace core {

        P2SHBitcoinLikeKeychain::P2SHBitcoinLikeKeychain(const std::shared_ptr<api::DynamicObject> &configuration,
                                                           const api::Currency &params,
                                                           int account,
                                                           const std::shared_ptr<api::BitcoinLikeExtendedPublicKey> &xpub,
                                                           const std::shared_ptr<Preferences> &preferences)
                : CommonBitcoinLikeKeychains(configuration, params, account, xpub, preferences)
        {
            getAllObservableAddresses(0, _observableRange);
        }

        std::string P2SHBitcoinLikeKeychain::derive(KeyPurpose purpose, off_t index) {

            auto iPurpose = (purpose == KeyPurpose::RECEIVE) ? 0 : 1;
            auto localPath = getDerivationScheme()
                    .setAccountIndex(getAccountIndex())
                    .setCoinType(getCurrency().bip44CoinType)
                    .setNode(iPurpose)
                    .setAddressIndex((int) index).getPath().toString();

            auto cacheKey = fmt::format("path:{}", localPath);
            auto address = getPreferences()->getString(cacheKey, "");

            if (address.empty()) {

                auto p = getDerivationScheme().getSchemeFrom(DerivationSchemeLevel::NODE).shift(1)
                        .setAccountIndex(getAccountIndex())
                        .setCoinType(getCurrency().bip44CoinType)
                        .setNode(iPurpose)
                        .setAddressIndex((int) index).getPath().toString();
                auto xpub = iPurpose == KeyPurpose::RECEIVE ? _publicNodeXpub : _internalNodeXpub;

                //Script
                std::vector<uint8_t> script = {0x00, 0x14};
                //Hash160 of public key
                auto publicKeyHash160 = xpub->deriveHash160(p);
                script.insert(script.end(), publicKeyHash160.begin(), publicKeyHash160.end());
                //Hash script
                auto hash160 = HASH160::hash(script);
                const auto& params = getCurrency().bitcoinLikeNetworkParameters.value();
                BitcoinLikeAddress btcLikeAddress(params, hash160, params.P2SHVersion);
                address = btcLikeAddress.toBase58();

                // Feed path -> address cache
                // Feed address -> path cache
                getPreferences()
                        ->edit()
                        ->putString(cacheKey, address)
                        ->putString(fmt::format("address:{}", address), localPath)
                        ->commit();
            }
            return address;
        }

        Option<std::string>
        P2SHBitcoinLikeKeychain::getHash160DerivationPath(const std::vector<uint8_t> &hash160) const {
            const auto& params = getCurrency().bitcoinLikeNetworkParameters.value();
            BitcoinLikeAddress address(params, hash160, params.P2SHVersion);
            return getAddressDerivationPath(address.toBase58());
        }

    }
}