/*
 *
 * EthereumLikeWalletFactory
 *
 * Created by El Khalil Bellakrid on 14/07/2018.
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

#include <core/Services.hpp>
#include <core/api/KeychainEngines.hpp>
#include <core/api/SynchronizationEngines.hpp>
#include <core/api/BlockchainExplorerEngines.hpp>
#include <core/api/BlockchainObserverEngines.hpp>
#include <core/api/ConfigurationDefaults.hpp>

#include <ethereum/EthereumLikeWallet.hpp>
#include <ethereum/EthereumNetworks.hpp>
#include <ethereum/database/Migrations.hpp>
#include <ethereum/explorers/LedgerApiEthereumLikeBlockchainExplorer.hpp>
#include <ethereum/factories/EthereumLikeWalletFactory.hpp>
#include <ethereum/factories/EthereumLikeKeychainFactory.hpp>
#include <ethereum/observers/LedgerApiEthereumLikeBlockchainObserver.hpp>
#include <ethereum/synchronizers/EthereumLikeBlockchainExplorerAccountSynchronizer.hpp>

#define STRING(key, def) entry.configuration->getString(key).value_or(def)

namespace ledger {
    namespace core {
        EthereumLikeWalletFactory::EthereumLikeWalletFactory(
            const api::Currency &currency,
            const std::shared_ptr<Services> &services
        ):
          AbstractWalletFactory(currency, services) {
            _keychainFactories = {{api::KeychainEngines::BIP49_P2SH, std::make_shared<EthereumLikeKeychainFactory>()}};
            // create the DB structure if not already created
            services->getDatabaseSessionPool()->forwardMigration<EthereumMigration>();

            // TODO: add currencies
        }

        std::shared_ptr<AbstractWallet> EthereumLikeWalletFactory::build(const WalletDatabaseEntry &entry)
        {
            auto services = getServices();
            services->logger()->info("Building wallet instance '{}' for {} with parameters: {}", entry.name, entry.currencyName, entry.configuration->dump());

            // Get currency
            auto currency = getCurrency();

            // Configure keychain
            auto keychainFactory = _keychainFactories.find(STRING(api::Configuration::KEYCHAIN_ENGINE, api::KeychainEngines::BIP49_P2SH));
            if (keychainFactory == _keychainFactories.end()) {
                throw make_exception(api::ErrorCode::UNKNOWN_KEYCHAIN_ENGINE, "Engine '{}' is not a supported keychain engine.", STRING(api::Configuration::KEYCHAIN_ENGINE, "undefined"));
            }
            // Configure explorer
            auto explorer = getExplorer(entry.currencyName, entry.configuration);
            if (explorer == nullptr)
                throw make_exception(api::ErrorCode::UNKNOWN_BLOCKCHAIN_EXPLORER_ENGINE, "Engine '{}' is not a supported explorer engine.", STRING(api::Configuration::BLOCKCHAIN_EXPLORER_ENGINE, "undefined"));
            // Configure observer
            auto observer = getObserver(entry.currencyName, entry.configuration);
            if (observer == nullptr)
                services->logger()->warn("Observer engine '{}' is not supported. Wallet {} was created anyway. Real time events won't be handled by this instance.",  STRING(api::Configuration::BLOCKCHAIN_OBSERVER_ENGINE, "undefined"), entry.name);
            // Configure synchronizer
            Option<EthereumLikeAccountSynchronizerFactory> synchronizerFactory;
            {
                auto engine = entry.configuration->getString(api::Configuration::SYNCHRONIZATION_ENGINE)
                        .value_or(api::SynchronizationEngines::BLOCKCHAIN_EXPLORER_SYNCHRONIZATION);
                if (engine == api::SynchronizationEngines::BLOCKCHAIN_EXPLORER_SYNCHRONIZATION) {
                    std::weak_ptr<Services> weakServices = services;
                    synchronizerFactory = Option<EthereumLikeAccountSynchronizerFactory>([weakServices, explorer]() {
                        auto services = weakServices.lock();
                        if (!services) {
                            throw make_exception(api::ErrorCode::NULL_POINTER, "Pool was released.");
                        }
                        return std::make_shared<EthereumLikeBlockchainExplorerAccountSynchronizer>(services, explorer);
                    });
                }
            }

            if (synchronizerFactory.isEmpty())
                throw make_exception(api::ErrorCode::UNKNOWN_SYNCHRONIZATION_ENGINE, "Engine '{}' is not a supported synchronization engine.", STRING(api::Configuration::SYNCHRONIZATION_ENGINE, "undefined"));
            // Sets the derivation scheme
            DerivationScheme scheme(STRING(api::Configuration::KEYCHAIN_DERIVATION_SCHEME, "44'/<coin_type>'/<account>'/<node>/<address>"));
            // Build wallet

            return std::make_shared<EthereumLikeWallet>(
                    entry.name,
                    explorer,
                    observer,
                    keychainFactory->second,
                    synchronizerFactory.getValue(),
                    services,
                    currency,
                    entry.configuration,
                    scheme
            );
        }

        std::shared_ptr<EthereumLikeBlockchainExplorer>
        EthereumLikeWalletFactory::getExplorer(const std::string& currencyName,
                                               const std::shared_ptr<api::DynamicObject>& configuration) {
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
                    .value_or(api::BlockchainExplorerEngines::LEDGER_API);
            std::shared_ptr<EthereumLikeBlockchainExplorer> explorer = nullptr;
            if (engine == api::BlockchainExplorerEngines::LEDGER_API) {
                auto http = services->getHttpClient(
                    configuration->getString(
                        api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT
                    ).value_or(api::ConfigurationDefaults::BLOCKCHAIN_DEFAULT_API_ENDPOINT)
                );
                auto currency = getCurrency();
                auto context = services->getDispatcher()->getSerialExecutionContext(
                    fmt::format("{}-{}-explorer",
                        api::BlockchainExplorerEngines::LEDGER_API,
                        currency.name
                    )
                );
                auto& networkParams = networks::getEthereumLikeNetworkParameters(currency.name);

                explorer = std::make_shared<LedgerApiEthereumLikeBlockchainExplorer>(context, http, networkParams, configuration);
            }
            if (explorer)
                _runningExplorers.push_back(explorer);
            return explorer;

        }

        std::shared_ptr<EthereumLikeBlockchainObserver>
        EthereumLikeWalletFactory::getObserver(const std::string& currencyName,
                                               const std::shared_ptr<api::DynamicObject>& configuration) {
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
                    .value_or(api::BlockchainObserverEngines::LEDGER_API);
            std::shared_ptr<EthereumLikeBlockchainObserver> observer;
            if (engine == api::BlockchainObserverEngines::LEDGER_API) {
                auto ws = services->getWebSocketClient();
                const auto& currency = getCurrency();
                auto context = services->getDispatcher()->getSerialExecutionContext(
                        fmt::format(
                            "{}-{}-observer",
                            api::BlockchainObserverEngines::LEDGER_API,
                            currency.name
                        )
                );
                auto logger = services->logger();
                observer = std::make_shared<LedgerApiEthereumLikeBlockchainObserver>(context, ws, configuration, logger, currency);
            }
            if (observer)
                _runningObservers.push_back(observer);
            return observer;
        }
    }
}
