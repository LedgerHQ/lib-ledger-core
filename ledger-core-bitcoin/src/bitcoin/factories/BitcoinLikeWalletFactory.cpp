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

#include <leveldb/db.h>

#include <core/Services.hpp>
#include <core/api/BlockchainExplorerEngines.hpp>
#include <core/api/ConfigurationDefaults.hpp>
#include <core/api/KeychainEngines.hpp>
#include <core/api/SynchronizationEngines.hpp>
#include <core/api/BlockchainObserverEngines.hpp>

#include <bitcoin/BitcoinNetworks.hpp>
#include <bitcoin/BitcoinLikeCurrencies.hpp>
#include <bitcoin/BitcoinLikeWallet.hpp>
#include <bitcoin/database/Migrations.hpp>
#include <bitcoin/explorers/LedgerApiBitcoinLikeBlockchainExplorer.hpp>
#include <bitcoin/factories/BitcoinLikeCommonKeychainFactory.hpp>
#include <bitcoin/factories/BitcoinLikeWalletFactory.hpp>
#include <bitcoin/keychains/P2PKHBitcoinLikeKeychain.hpp>
#include <bitcoin/keychains/P2SHBitcoinLikeKeychain.hpp>
#include <bitcoin/keychains/P2WPKHBitcoinLikeKeychain.hpp>
#include <bitcoin/keychains/P2WSHBitcoinLikeKeychain.hpp>
#include <bitcoin/observers/LedgerApiBitcoinLikeBlockchainObserver.hpp>
#include <bitcoin/synchronizers/BitcoinLikeBlockchainExplorerAccountSynchronizer.hpp>

#define STRING(key, def) entry.configuration->getString(key).value_or(def)

namespace ledger {
    namespace core {
        BitcoinLikeWalletFactory::BitcoinLikeWalletFactory(
            const api::Currency &currency,
            const std::shared_ptr<Services> &services
        ): AbstractWalletFactory(currency, services) {
            _keychainFactories = {
                {api::KeychainEngines::BIP32_P2PKH, std::make_shared<BitcoinLikeCommonKeychainFactory<P2PKHBitcoinLikeKeychain>>()},
                {api::KeychainEngines::BIP49_P2SH, std::make_shared<BitcoinLikeCommonKeychainFactory<P2SHBitcoinLikeKeychain>>()},
                {api::KeychainEngines::BIP173_P2WPKH, std::make_shared<BitcoinLikeCommonKeychainFactory<P2WPKHBitcoinLikeKeychain>>()},
                {api::KeychainEngines::BIP173_P2WSH, std::make_shared<BitcoinLikeCommonKeychainFactory<P2WSHBitcoinLikeKeychain>>()}
            };

            services->getDatabaseSessionPool()->forwardMigration<BitcoinMigration>();
        }

        std::shared_ptr<AbstractWallet> BitcoinLikeWalletFactory::build(const WalletDatabaseEntry &entry) {
            auto services = getServices();
            services->logger()->info("Building wallet instance '{}' for {} with parameters: {}", entry.name, entry.currencyName, entry.configuration->dump());

            // Get currency
            try {
                auto currency = networks::getBitcoinLikeNetworkParameters(entry.currencyName);
            }
            catch (Exception) {
                throw make_exception(api::ErrorCode::UNSUPPORTED_CURRENCY, "Unsupported currency '{}'.", entry.currencyName);
            }

            // Configure keychain
            auto keychainFactory = _keychainFactories.find(STRING(api::Configuration::KEYCHAIN_ENGINE, api::KeychainEngines::BIP32_P2PKH));
            if (keychainFactory == _keychainFactories.end()) {
                throw make_exception(api::ErrorCode::UNKNOWN_KEYCHAIN_ENGINE, "Engine '{}' is not a supported keychain engine.", STRING(api::Configuration::KEYCHAIN_ENGINE, "undefined"));
            }

            // Configure explorer
            auto explorer = getExplorer(entry.currencyName, entry.configuration);
            if (explorer == nullptr) {
                throw make_exception(api::ErrorCode::UNKNOWN_BLOCKCHAIN_EXPLORER_ENGINE, "Engine '{}' is not a supported explorer engine.", STRING(api::Configuration::BLOCKCHAIN_EXPLORER_ENGINE, "undefined"));
            }

            // Configure observer
            auto observer = getObserver(entry.currencyName, entry.configuration);
            if (observer == nullptr) {
                services->logger()->warn("Observer engine '{}' is not supported. Wallet {} was created anyway. Real time events won't be handled by this instance.",  STRING(api::Configuration::BLOCKCHAIN_OBSERVER_ENGINE, "undefined"), entry.name);
            }

            // Configure synchronizer
            using BitcoinLikeAccountSynchronizerFactory = std::function<std::shared_ptr<BitcoinLikeAccountSynchronizer> ()>;
            Option<BitcoinLikeAccountSynchronizerFactory> synchronizerFactory;

            {
                auto engine = entry.configuration->getString(api::Configuration::SYNCHRONIZATION_ENGINE)
                                           .value_or(api::SynchronizationEngines::BLOCKCHAIN_EXPLORER_SYNCHRONIZATION);
                if (engine == api::SynchronizationEngines::BLOCKCHAIN_EXPLORER_SYNCHRONIZATION) {
                    std::weak_ptr<Services> s = services;
                    synchronizerFactory = Option<BitcoinLikeAccountSynchronizerFactory>([s, explorer]() {
                        auto services = s.lock();
                        if (!services) {
                            throw make_exception(api::ErrorCode::NULL_POINTER, "WalletPool was released.");
                        }
                        return std::make_shared<BlockchainExplorerAccountSynchronizer>(services, explorer);
                    });
                }
            }

            if (synchronizerFactory.isEmpty()) {
                throw make_exception(api::ErrorCode::UNKNOWN_SYNCHRONIZATION_ENGINE, "Engine '{}' is not a supported synchronization engine.", STRING(api::Configuration::SYNCHRONIZATION_ENGINE, "undefined"));
            }

            // Sets the derivation scheme
            DerivationScheme scheme(STRING(api::Configuration::KEYCHAIN_DERIVATION_SCHEME, "44'/<coin_type>'/<account>'/<node>/<address>"));

            return std::make_shared<BitcoinLikeWallet>(
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

        std::shared_ptr<BitcoinLikeBlockchainExplorer>
        BitcoinLikeWalletFactory::getExplorer(const std::string &currencyName,
                                              const std::shared_ptr<api::DynamicObject> &configuration) {
            auto it = _runningExplorers.begin();
            while (it != _runningExplorers.end()) {
                auto explorer = it->lock();
                if (explorer != nullptr) {
                    if (explorer->match(configuration)) {
                        return explorer;
                    }

                    it++;
                } else {
                    it = _runningExplorers.erase(it);
                }
            }

            auto services = getServices();
            auto engine = configuration->getString(api::Configuration::BLOCKCHAIN_EXPLORER_ENGINE)
                                       .value_or(api::BlockchainExplorerEngines::LEDGER_API);
            std::shared_ptr<BitcoinLikeBlockchainExplorer> explorer = nullptr;
            if (engine == api::BlockchainExplorerEngines::LEDGER_API) {
                auto http = services->getHttpClient(
                        configuration->getString(
                                api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT
                        ).value_or(api::ConfigurationDefaults::BLOCKCHAIN_DEFAULT_API_ENDPOINT)
                );

                auto context = services->getDispatcher()->getSerialExecutionContext(api::BlockchainObserverEngines::LEDGER_API);
                auto& networkParams = networks::getBitcoinLikeNetworkParameters(getCurrency().name);
                explorer = std::make_shared<LedgerApiBitcoinLikeBlockchainExplorer>(context, http, networkParams, configuration);
            }

            if (explorer) {
                _runningExplorers.push_back(explorer);
            }

            return explorer;
        }

        std::shared_ptr<BitcoinLikeBlockchainObserver>
        BitcoinLikeWalletFactory::getObserver(const std::string &currencyName,
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
                                        .value_or(api::BlockchainObserverEngines::LEDGER_API);
            std::shared_ptr<BitcoinLikeBlockchainObserver> observer;

            if (engine == api::BlockchainObserverEngines::LEDGER_API) {
                auto ws = services->getWebSocketClient();
                auto context = services->getDispatcher()->getSerialExecutionContext(api::BlockchainObserverEngines::LEDGER_API);
                auto logger = services->logger();
                const auto& currency = getCurrency();

                observer = std::make_shared<LedgerApiBitcoinLikeBlockchainObserver>(
                        context,
                        ws,
                        configuration,
                        logger,
                        currency
                );
            }

            if (observer) {
                _runningObservers.push_back(observer);
            }

            return observer;
        }
    }
}
