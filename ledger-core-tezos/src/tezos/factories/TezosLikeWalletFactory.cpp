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


#include <tezos/TezosLikeWallet.hpp>
#include <tezos/TezosNetworks.hpp>
#include <tezos/api/TezosConfigurationDefaults.hpp>
#include <tezos/api/TezosBlockchainExplorerEngines.hpp>
#include <tezos/api/TezosBlockchainObserverEngines.hpp>
#include <tezos/explorers/NodeTezosLikeBlockchainExplorer.hpp>
#include <tezos/explorers/ExternalTezosLikeBlockchainExplorer.hpp>
#include <tezos/factories/TezosLikeWalletFactory.hpp>
#include <tezos/factories/TezosLikeKeychainFactory.hpp>
#include <tezos/observers/TezosLikeBlockchainObserver.hpp>
#include <tezos/synchronizers/TezosLikeBlockchainExplorerAccountSynchronizer.hpp>

#include <core/Services.hpp>
#include <core/api/KeychainEngines.hpp>
#include <core/api/SynchronizationEngines.hpp>
#include <core/api/BlockchainExplorerEngines.hpp>
#include <core/api/BlockchainObserverEngines.hpp>
#include <core/api/ConfigurationDefaults.hpp>


#define STRING(key, def) entry.configuration->getString(key).value_or(def)

namespace ledger {
    namespace core {
        TezosLikeWalletFactory::TezosLikeWalletFactory(const api::Currency &currency,
                                                       const std::shared_ptr<Services> &services) :
                AbstractWalletFactory(currency, services) {
            _keychainFactories = {{api::KeychainEngines::BIP49_P2SH, std::make_shared<TezosLikeKeychainFactory>()}};
        }

        std::shared_ptr<AbstractWallet> TezosLikeWalletFactory::build(const WalletDatabaseEntry &entry) {
            auto services = getServices();
            services->logger()->info("Building wallet instance '{}' for {} with parameters: {}", entry.name,
                                 entry.currencyName, entry.configuration->dump());

            // Get currency
            auto isSupportedCurrency = [entry](auto const& networkParameters) {
                return entry.currencyName == networkParameters.Identifier;
            };

            if (std::none_of(std::begin(networks::ALL_TEZOS), std::end(networks::ALL_TEZOS), isSupportedCurrency)) {
                throw make_exception(api::ErrorCode::UNSUPPORTED_CURRENCY, "Unsupported currency '{}'.", entry.currencyName);
            }

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
                services->logger()->warn(
                        "Observer engine '{}' is not supported. Wallet {} was created anyway. Real time events won't be handled by this instance.",
                        STRING(api::Configuration::BLOCKCHAIN_OBSERVER_ENGINE, "undefined"), entry.name);
            // Configure synchronizer
            Option<TezosLikeAccountSynchronizerFactory> synchronizerFactory;
            {
                auto engine = entry.configuration->getString(api::Configuration::SYNCHRONIZATION_ENGINE)
                        .value_or(api::SynchronizationEngines::BLOCKCHAIN_EXPLORER_SYNCHRONIZATION);
                if (engine == api::SynchronizationEngines::BLOCKCHAIN_EXPLORER_SYNCHRONIZATION) {
                    std::weak_ptr<Services> weakServices = services;
                    synchronizerFactory = Option<TezosLikeAccountSynchronizerFactory>([weakServices, explorer]() {
                        auto services = weakServices.lock();
                        if (!services) {
                            throw make_exception(api::ErrorCode::NULL_POINTER, "Pool was released.");
                        }
                        return std::make_shared<TezosLikeBlockchainExplorerAccountSynchronizer>(services, explorer);
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
                    services,
                    getCurrency(),
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

            auto services = getServices();
            auto engine = configuration->getString(api::Configuration::BLOCKCHAIN_EXPLORER_ENGINE)
                    .value_or(api::TezosBlockchainExplorerEngines::TEZOS_NODE);
            std::shared_ptr<TezosLikeBlockchainExplorer> explorer = nullptr;
            auto isTzStats = engine == api::TezosBlockchainExplorerEngines::TZSTATS_API;
            if (engine == api::TezosBlockchainExplorerEngines::TEZOS_NODE ||
                    isTzStats) {
                auto defaultValue = isTzStats ?
                        api::TezosConfigurationDefaults::TZSTATS_API_ENDPOINT :
                                    api::TezosConfigurationDefaults::TEZOS_DEFAULT_API_ENDPOINT;
                auto http = services->getHttpClient(fmt::format("{}",
                                                            configuration->getString(
                                                                    api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT
                                                            ).value_or(defaultValue)));
                auto &networkParams = networks::getTezosLikeNetworkParameters(getCurrency().name);
                auto context = services->getDispatcher()->getSerialExecutionContext(
                        fmt::format("{}-{}-explorer",
                                    api::TezosBlockchainExplorerEngines::TEZOS_NODE,
                                    networkParams.Identifier
                        )
                );

                if (isTzStats) {
                    explorer = std::make_shared<ExternalTezosLikeBlockchainExplorer>(context,
                                                                                     http,
                                                                                     networkParams,
                                                                                     configuration);
                } else {
                    explorer = std::make_shared<NodeTezosLikeBlockchainExplorer>(context,
                                                                                 http,
                                                                                 networkParams,
                                                                                 configuration);
                }
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

            auto services = getServices();
            auto engine = configuration->getString(api::Configuration::BLOCKCHAIN_OBSERVER_ENGINE)
                    .value_or(api::TezosBlockchainObserverEngines::TEZOS_NODE);
            std::shared_ptr<TezosLikeBlockchainObserver> observer;
            if (engine == api::TezosBlockchainObserverEngines::TEZOS_NODE) {
                auto ws = services->getWebSocketClient();
                const auto &currency = getCurrency();
                auto &networkParams = networks::getTezosLikeNetworkParameters(currency.name);
                auto context = services->getDispatcher()->getSerialExecutionContext(
                        fmt::format("{}-{}-explorer",
                                    api::TezosBlockchainObserverEngines::TEZOS_NODE,
                                    networkParams.Identifier
                        )
                );
                auto logger = services->logger();
                observer = std::make_shared<TezosLikeBlockchainObserver>(context, ws, configuration, logger, currency);
            }
            if (observer)
                _runningObservers.push_back(observer);
            return observer;
        }
    }
}
