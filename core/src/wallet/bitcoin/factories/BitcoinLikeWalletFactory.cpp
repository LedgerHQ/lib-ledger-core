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
#include "../networks.hpp"
#include <leveldb/db.h>

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
                                        std::shared_ptr<api::DynamicObject> configuration) {
            Promise<std::shared_ptr<BitcoinLikeWallet>> promise;
            BitcoinLikeWalletEntry entry;
            std::string identifier = "";
            entry.configuration = std::static_pointer_cast<DynamicObject>(configuration);
            _preferences->editor()->putObject(identifier, entry)->commit();
            return build(entry);
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

        void BitcoinLikeWalletFactory::initialize(const std::shared_ptr<Preferences> &preferences,
                                                  Map<std::string, ledger::core::api::BitcoinLikeNetworkParameters> &networks) {
            for (auto network : ledger::core::networks::ALL) {
                networks[network.Identifier] = network;
            }
            if (!preferences->contains(ledger::core::networks::BITCOIN.Identifier)) {
                auto editor = preferences->editor();
                for (auto network : ledger::core::networks::ALL) {
                    editor->putObject<api::BitcoinLikeNetworkParameters>(network.Identifier, network);
                }
                editor->commit();
            }
            preferences->iterate<api::BitcoinLikeNetworkParameters>([&] (leveldb::Slice&& key, const api::BitcoinLikeNetworkParameters& value) {
                networks[value.Identifier] = value;
                return true;
            });
        }

    }
}