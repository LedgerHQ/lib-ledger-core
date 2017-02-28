/*
 *
 * BitcoinLikeWalletFactory
 * ledger-core
 *
 * Created by Pierre Pollastri on 31/01/2017.
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
#include "BitcoinLikeWalletFactory.hpp"

namespace ledger {
    namespace core {

        BitcoinLikeWalletFactory::BitcoinLikeWalletFactory(const api::BitcoinLikeNetworkParameters &params,
                                                           std::shared_ptr<WalletPool> pool,
                                                           std::shared_ptr<Preferences> preferences) : _params(params) {
            _preferences = preferences;
            _pool = pool;
        }

        Future<std::shared_ptr<BitcoinLikeWallet>>
        BitcoinLikeWalletFactory::build(std::shared_ptr<api::BitcoinLikeExtendedPublicKeyProvider> provider,
                                        std::shared_ptr<api::Configuration> configuration) {
            Promise<std::shared_ptr<BitcoinLikeWallet>> promise;
            BitcoinLikeWalletEntry entry;
            std::string identifier = "";
            entry.configuration = std::static_pointer_cast<ConfigurationImpl>(configuration);
            _preferences->editor()->putObject(identifier, entry)->commit();
            return promise.getFuture();
        }

        Future<std::shared_ptr<BitcoinLikeWallet>>
        BitcoinLikeWalletFactory::build(const std::string identifier) {
            return  _preferences->getObject<BitcoinLikeWalletEntry>(identifier)
                    .map<Future<std::shared_ptr<BitcoinLikeWallet>>>([=] (const BitcoinLikeWalletEntry& entry) {
                        return build(entry);
                    })
                    .getValueOr(Future<std::shared_ptr<BitcoinLikeWallet>>::failure(Exception(api::ErrorCode::WALLET_NOT_FOUND, "Wallet not found")));
        }

        Future<std::shared_ptr<BitcoinLikeWallet>>
        BitcoinLikeWalletFactory::build(const BitcoinLikeWalletEntry& entry) {
            Promise<std::shared_ptr<BitcoinLikeWallet>> promise;
            return promise.getFuture();
        }

    }
}