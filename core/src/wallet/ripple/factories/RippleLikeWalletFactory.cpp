/*
 *
 * RippleLikeWalletFactory
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

#include "RippleLikeWalletFactory.h"

#include <api/BlockchainExplorerEngines.hpp>
#include <api/ConfigurationDefaults.hpp>
#include <api/KeychainEngines.hpp>
#include <api/RippleConfigurationDefaults.hpp>
#include <api/SynchronizationEngines.hpp>
#include <wallet/pool/WalletPool.hpp>
#include <wallet/ripple/RippleLikeWallet.h>
#include <wallet/ripple/explorers/ApiRippleLikeBlockchainExplorer.h>
#include <wallet/ripple/explorers/NodeRippleLikeBlockchainExplorer.h>
#include <wallet/ripple/factories/RippleLikeKeychainFactory.h>

#define STRING(key, def) entry.configuration->getString(key).value_or(def)

namespace ledger {
    namespace core {
        RippleLikeWalletFactory::RippleLikeWalletFactory(const api::Currency &currency,
                                                         const std::shared_ptr<WalletPool> &pool) : AbstractWalletFactory(currency, pool) {
            _keychainFactories = {{api::KeychainEngines::BIP49_P2SH, std::make_shared<RippleLikeKeychainFactory>()}};
        }

        std::shared_ptr<AbstractWallet> RippleLikeWalletFactory::build(const WalletDatabaseEntry &entry) {
            auto pool = getPool();
            pool->logger()->info("Building wallet instance '{}' for {} with parameters: {}", entry.name, entry.currencyName, entry.configuration->dump());
            // Get currency
            auto currency = getPool()->getCurrency(entry.currencyName);
            if (currency.isEmpty())
                throw make_exception(api::ErrorCode::UNSUPPORTED_CURRENCY, "Unsupported currency '{}'.", entry.currencyName);
            // Configure keychain
            auto keychainFactory = _keychainFactories.find(STRING(api::Configuration::KEYCHAIN_ENGINE, api::KeychainEngines::BIP49_P2SH));
            if (keychainFactory == _keychainFactories.end()) {
                throw make_exception(api::ErrorCode::UNKNOWN_KEYCHAIN_ENGINE, "Engine '{}' is not a supported keychain engine.", STRING(api::Configuration::KEYCHAIN_ENGINE, "undefined"));
            }
            // Configure explorer
            auto explorer = getExplorer(entry.currencyName, entry.configuration);
            if (explorer == nullptr)
                throw make_exception(api::ErrorCode::UNKNOWN_BLOCKCHAIN_EXPLORER_ENGINE, "Engine '{}' is not a supported explorer engine.", STRING(api::Configuration::BLOCKCHAIN_EXPLORER_ENGINE, "undefined"));
            // Configure synchronizer
            Option<RippleLikeAccountSynchronizerFactory> synchronizerFactory;
            {
                auto engine = entry.configuration->getString(api::Configuration::SYNCHRONIZATION_ENGINE)
                                  .value_or(api::SynchronizationEngines::BLOCKCHAIN_EXPLORER_SYNCHRONIZATION);
                if (engine == api::SynchronizationEngines::BLOCKCHAIN_EXPLORER_SYNCHRONIZATION) {
                    std::weak_ptr<WalletPool> p = pool;
                    synchronizerFactory = Option<RippleLikeAccountSynchronizerFactory>([p, explorer]() {
                        auto pool = p.lock();
                        return std::make_shared<RippleLikeAccountSynchronizer>(pool, explorer);
                    });
                }
            }

            if (synchronizerFactory.isEmpty())
                throw make_exception(api::ErrorCode::UNKNOWN_SYNCHRONIZATION_ENGINE, "Engine '{}' is not a supported synchronization engine.", STRING(api::Configuration::SYNCHRONIZATION_ENGINE, "undefined"));
            // Sets the derivation scheme
            DerivationScheme scheme(STRING(api::Configuration::KEYCHAIN_DERIVATION_SCHEME, "44'/<coin_type>'/<account>'/<node>/<address>"));
            // Build wallet

            return std::make_shared<RippleLikeWallet>(
                entry.name,
                explorer,
                keychainFactory->second,
                synchronizerFactory.getValue(),
                pool,
                currency.getValue(),
                entry.configuration,
                scheme);
        }

        std::shared_ptr<RippleLikeBlockchainExplorer>
        RippleLikeWalletFactory::getExplorer(const std::string &currencyName,
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
                              .value_or(api::BlockchainExplorerEngines::RIPPLE_NODE);
            std::shared_ptr<RippleLikeBlockchainExplorer> explorer = nullptr;
            auto &networkParams = getCurrency().rippleLikeNetworkParameters.value();
            auto context = pool->getDispatcher()->getSerialExecutionContext(
                fmt::format("{}-{}-explorer",
                            api::BlockchainExplorerEngines::RIPPLE_NODE,
                            networkParams.Identifier));
            if (engine == api::BlockchainExplorerEngines::RIPPLE_NODE) {
                auto http = pool->getHttpClient(
                    fmt::format("{}:{}",
                                configuration->getString(api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT)
                                    .value_or(api::RippleConfigurationDefaults::RIPPLE_OBSERVER_NODE_ENDPOINT_S2),
                                configuration->getString(api::Configuration::BLOCKCHAIN_EXPLORER_PORT)
                                    .value_or(api::RippleConfigurationDefaults::RIPPLE_DEFAULT_PORT)));

                explorer = std::make_shared<NodeRippleLikeBlockchainExplorer>(context, http, networkParams, configuration);
            } else if (engine == api::BlockchainExplorerEngines::RIPPLE_API) {
                auto http = pool->getHttpClient(
                    configuration->getString(
                                     api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT)
                        .value_or(api::RippleConfigurationDefaults::RIPPLE_DEFAULT_API_ENDPOINT));
                explorer = std::make_shared<ApiRippleLikeBlockchainExplorer>(context, http, networkParams, configuration);
            }
            if (explorer)
                _runningExplorers.push_back(explorer);
            return explorer;
        }
    } // namespace core
} // namespace ledger
