/*
 *
 * TezosLikeWalletFactory
 *
 * Created by El Khalil Bellakrid on 27/04/2019.
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


#include "TezosLikeWalletFactory.h"
#include <api/KeychainEngines.hpp>
#include <api/SynchronizationEngines.hpp>
#include <api/BlockchainExplorerEngines.hpp>
#include <api/BlockchainObserverEngines.hpp>
#include <api/ConfigurationDefaults.hpp>

#include <wallet/tezos/explorers/NodeTezosLikeBlockchainExplorer.h>
#include <wallet/tezos/factories/TezosLikeKeychainFactory.h>
#include <wallet/tezos/TezosLikeWallet.h>
#include <wallet/pool/WalletPool.hpp>
#include <wallet/tezos/synchronizers/TezosLikeBlockchainExplorerAccountSynchronizer.h>
#include <wallet/tezos/observers/TezosLikeBlockchainObserver.h>
#include <api/TezosConfigurationDefaults.hpp>

#define STRING(key, def) entry.configuration->getString(key).value_or(def)

namespace ledger {
    namespace core {
        TezosLikeWalletFactory::TezosLikeWalletFactory(const api::Currency &currency,
                                                       const std::shared_ptr<WalletPool> &pool) :
                AbstractWalletFactory(currency, pool) {
            _keychainFactories = {{api::KeychainEngines::BIP49_P2SH, std::make_shared<TezosLikeKeychainFactory>()}};
        }

        std::shared_ptr<AbstractWallet> TezosLikeWalletFactory::build(const WalletDatabaseEntry &entry) {
            auto pool = getPool();
            pool->logger()->info("Building wallet instance '{}' for {} with parameters: {}", entry.name,
                                 entry.currencyName, entry.configuration->dump());
            // Get currency
            auto currency = getPool()->getCurrency(entry.currencyName);
            if (currency.isEmpty())
                throw make_exception(api::ErrorCode::UNSUPPORTED_CURRENCY, "Unsupported currency '{}'.",
                                     entry.currencyName);
            // Configure keychain
            auto keychainFactory = _keychainFactories.find(
                    STRING(api::Configuration::KEYCHAIN_ENGINE, api::KeychainEngines::BIP49_P2SH));
            if (keychainFactory == _keychainFactories.end()) {
                throw make_exception(api::ErrorCode::UNKNOWN_KEYCHAIN_ENGINE,
                                     "Engine '{}' is not a supported keychain engine.",
                                     STRING(api::Configuration::KEYCHAIN_ENGINE, "undefined"));
            }
            // Configure explorer
            auto explorer = getExplorer(entry.currencyName, entry.configuration);
            if (explorer == nullptr)
                throw make_exception(api::ErrorCode::UNKNOWN_BLOCKCHAIN_EXPLORER_ENGINE,
                                     "Engine '{}' is not a supported explorer engine.",
                                     STRING(api::Configuration::BLOCKCHAIN_EXPLORER_ENGINE, "undefined"));
            // Configure observer
            auto observer = getObserver(entry.currencyName, entry.configuration);
            if (observer == nullptr)
                pool->logger()->warn(
                        "Observer engine '{}' is not supported. Wallet {} was created anyway. Real time events won't be handled by this instance.",
                        STRING(api::Configuration::BLOCKCHAIN_OBSERVER_ENGINE, "undefined"), entry.name);
            // Configure synchronizer
            Option<TezosLikeAccountSynchronizerFactory> synchronizerFactory;
            {
                auto engine = entry.configuration->getString(api::Configuration::SYNCHRONIZATION_ENGINE)
                        .value_or(api::SynchronizationEngines::BLOCKCHAIN_EXPLORER_SYNCHRONIZATION);
                if (engine == api::SynchronizationEngines::BLOCKCHAIN_EXPLORER_SYNCHRONIZATION) {
                    std::weak_ptr<WalletPool> p = pool;
                    synchronizerFactory = Option<TezosLikeAccountSynchronizerFactory>([p, explorer]() {
                        auto pool = p.lock();
                        return std::make_shared<TezosLikeBlockchainExplorerAccountSynchronizer>(pool, explorer);
                    });
                }
            }

            if (synchronizerFactory.isEmpty())
                throw make_exception(api::ErrorCode::UNKNOWN_SYNCHRONIZATION_ENGINE,
                                     "Engine '{}' is not a supported synchronization engine.",
                                     STRING(api::Configuration::SYNCHRONIZATION_ENGINE, "undefined"));
            // Sets the derivation scheme
            DerivationScheme scheme(STRING(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,
                                           "44'/<coin_type>'/<account>'/<node>/<address>"));
            // Build wallet

            return std::make_shared<TezosLikeWallet>(
                    entry.name,
                    explorer,
                    observer,
                    keychainFactory->second,
                    synchronizerFactory.getValue(),
                    pool,
                    currency.getValue(),
                    entry.configuration,
                    scheme
            );
        }

        std::shared_ptr<TezosLikeBlockchainExplorer>
        TezosLikeWalletFactory::getExplorer(const std::string &currencyName,
                                            const std::shared_ptr<api::DynamicObject> &configuration) {
            auto it = _runningExplorers.begin();
            while (it != _runningExplorers.end()) {
                auto explorer = it->lock();
                if (explorer != nullptr) {
                    if (explorer->match(configuration))
                        return explorer;
                    it++;
                } else {
                    it = _runningExplorers.erase(it);
                }
            }

            auto pool = getPool();
            auto engine = configuration->getString(api::Configuration::BLOCKCHAIN_EXPLORER_ENGINE)
                    .value_or(api::BlockchainExplorerEngines::TEZOS_NODE);
            std::shared_ptr<TezosLikeBlockchainExplorer> explorer = nullptr;
            if (engine == api::BlockchainExplorerEngines::TEZOS_NODE) {
                auto http = pool->getHttpClient(fmt::format("{}/{}",
                                                            configuration->getString(
                                                                    api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT
                                                            ).value_or(
                                                                    api::TezosConfigurationDefaults::TEZOS_DEFAULT_API_ENDPOINT),
                                                configuration->getString(api::Configuration::BLOCKCHAIN_EXPLORER_VERSION).value_or(api::TezosConfigurationDefaults::TEZOS_DEFAULT_API_VERSION))
                );
                auto context = pool->getDispatcher()->getSerialExecutionContext(
                        api::BlockchainObserverEngines::TEZOS_NODE);
                auto &networkParams = getCurrency().tezosLikeNetworkParameters.value();

                explorer = std::make_shared<NodeTezosLikeBlockchainExplorer>(context, http, networkParams,
                                                                             configuration);
            }
            if (explorer)
                _runningExplorers.push_back(explorer);
            return explorer;

        }

        std::shared_ptr<TezosLikeBlockchainObserver>
        TezosLikeWalletFactory::getObserver(const std::string &currencyName,
                                            const std::shared_ptr<api::DynamicObject> &configuration) {
            auto it = _runningObservers.begin();
            while (it != _runningObservers.end()) {
                auto observer = it->lock();
                if (observer != nullptr) {
                    if (observer->match(configuration)) {
                        return observer;
                    }
                    it++;
                } else {
                    it = _runningObservers.erase(it);
                }
            }

            auto pool = getPool();
            auto engine = configuration->getString(api::Configuration::BLOCKCHAIN_OBSERVER_ENGINE)
                    .value_or(api::BlockchainObserverEngines::TEZOS_NODE);
            std::shared_ptr<TezosLikeBlockchainObserver> observer;
            if (engine == api::BlockchainObserverEngines::TEZOS_NODE) {
                auto ws = pool->getWebSocketClient();
                auto context = pool->getDispatcher()->getSerialExecutionContext(
                        api::BlockchainObserverEngines::TEZOS_NODE);
                auto logger = pool->logger();
                const auto &currency = getCurrency();
                observer = std::make_shared<TezosLikeBlockchainObserver>(context, ws, configuration, logger, currency);
            }
            if (observer)
                _runningObservers.push_back(observer);
            return observer;
        }
    }
}