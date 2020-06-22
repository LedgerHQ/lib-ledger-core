/*
 *
 * CosmosLikeWalletFactory
 *
 * Created by El Khalil Bellakrid on 14/06/2019.
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
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
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

#include <core/api/ConfigurationDefaults.hpp>
#include <core/api/CosmosConfigurationDefaults.hpp>
#include <core/api/KeychainEngines.hpp>
#include <core/api/SynchronizationEngines.hpp>
#include <cosmos/CosmosNetworks.hpp>
#include <cosmos/Wallet.hpp>
#include <cosmos/api/CosmosBlockchainExplorerEngines.hpp>
#include <cosmos/api/CosmosBlockchainObserverEngines.hpp>
#include <cosmos/database/Migrations.hpp>
#include <cosmos/explorers/GaiaBlockchainExplorer.hpp>
#include <cosmos/factories/KeychainFactory.hpp>
#include <cosmos/factories/WalletFactory.hpp>
#include <cosmos/observers/BlockchainObserver.hpp>
#include <cosmos/synchronizers/AccountSynchronizer.hpp>

#define STRING(key, def) entry.configuration->getString(key).value_or(def)

namespace ledger {
namespace core {
namespace cosmos {
CosmosLikeWalletFactory::CosmosLikeWalletFactory(
    const api::Currency &currency, const std::shared_ptr<Services> &services)
    : AbstractWalletFactory(currency, services) {
  _keychainFactories = {{api::KeychainEngines::BIP49_P2SH,
                         std::make_shared<CosmosLikeKeychainFactory>()}};

  services->getDatabaseSessionPool()->forwardMigration<CosmosMigration>();
}

std::shared_ptr<AbstractWallet>
CosmosLikeWalletFactory::build(const WalletDatabaseEntry &entry) {
  auto services = getServices();
  services->logger()->info(
      "Building wallet instance '{}' for {} with parameters: {}", entry.name,
      entry.currencyName, entry.configuration->dump());
  // Get currency
  try {
    const auto currency =
        networks::getNetworkParameters(entry.currencyName);
  } catch (Exception) {
    throw make_exception(api::ErrorCode::UNSUPPORTED_CURRENCY,
                         "Unsupported Cosmos family currency '{}'.",
                         entry.currencyName);
  }

  // Configure keychain
  auto keychainFactory = _keychainFactories.find(STRING(
      api::Configuration::KEYCHAIN_ENGINE, api::KeychainEngines::BIP49_P2SH));
  if (keychainFactory == _keychainFactories.end()) {
    throw make_exception(
        api::ErrorCode::UNKNOWN_KEYCHAIN_ENGINE,
        "Engine '{}' is not a supported Cosmos family keychain engine.",
        STRING(api::Configuration::KEYCHAIN_ENGINE, "undefined"));
  }
  // Configure explorer
  auto explorer = getExplorer(entry.currencyName, entry.configuration);
  if (explorer == nullptr)
    throw make_exception(
        api::ErrorCode::UNKNOWN_BLOCKCHAIN_EXPLORER_ENGINE,
        "Engine '{}' is not a supported Cosmos family explorer engine.",
        STRING(api::Configuration::BLOCKCHAIN_EXPLORER_ENGINE, "undefined"));
  // Configure observer
  auto observer = getObserver(entry.currencyName, entry.configuration);
  if (observer == nullptr)
    services->logger()->warn(
        "Observer engine '{}' is not supported. Wallet {} was created anyway. "
        "Real time events "
        "won't be handled by this instance.",
        STRING(api::Configuration::BLOCKCHAIN_OBSERVER_ENGINE, "undefined"),
        entry.name);
  // Configure synchronizer
  Option<CosmosLikeAccountSynchronizerFactory> synchronizerFactory;
  {
    auto engine = entry.configuration
                      ->getString(api::Configuration::SYNCHRONIZATION_ENGINE)
                      .value_or(api::SynchronizationEngines::
                                    BLOCKCHAIN_EXPLORER_SYNCHRONIZATION);
    if (engine ==
        api::SynchronizationEngines::BLOCKCHAIN_EXPLORER_SYNCHRONIZATION) {
      std::weak_ptr<Services> s = services;
      synchronizerFactory =
          Option<CosmosLikeAccountSynchronizerFactory>([s, explorer]() {
            auto services = s.lock();
            return std::make_shared<CosmosLikeAccountSynchronizer>(services,
                                                                   explorer);
          });
    }
  }

  if (synchronizerFactory.isEmpty())
    throw make_exception(
        api::ErrorCode::UNKNOWN_SYNCHRONIZATION_ENGINE,
        "Engine '{}' is not a supported synchronization engine.",
        STRING(api::Configuration::SYNCHRONIZATION_ENGINE, "undefined"));
  // Sets the derivation scheme
  DerivationScheme scheme(
      STRING(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,
             "44'/<coin_type>'/<account>'/<node>/<address>"));
  // Build wallet

  return std::make_shared<CosmosLikeWallet>(
      entry.name, explorer, observer, keychainFactory->second,
      synchronizerFactory.getValue(), services, getCurrency(),
      entry.configuration, scheme);
}

std::shared_ptr<CosmosLikeBlockchainExplorer>
CosmosLikeWalletFactory::getExplorer(
    const std::string &currencyName,
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
  auto engine =
      configuration->getString(api::Configuration::BLOCKCHAIN_EXPLORER_ENGINE)
          .value_or(api::CosmosBlockchainExplorerEngines::COSMOS_NODE);
  std::shared_ptr<CosmosLikeBlockchainExplorer> explorer = nullptr;
  if (engine == api::CosmosBlockchainExplorerEngines::COSMOS_NODE) {
    auto http = services->getHttpClient(fmt::format(
        "{}",
        configuration
            ->getString(api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT)
            .value_or(api::CosmosConfigurationDefaults::
                          COSMOS_DEFAULT_API_ENDPOINT)));
    auto context = services->getDispatcher()->getSerialExecutionContext(
        api::CosmosBlockchainObserverEngines::COSMOS_NODE);
    auto &networkParams = networks::getNetworkParameters(getCurrency().name);

    explorer = std::make_shared<GaiaCosmosLikeBlockchainExplorer>(
        context, http, networkParams,
        std::dynamic_pointer_cast<DynamicObject>(configuration));
  } else {
    throw Exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING,
                    "CosmosLikeWalletFactory using non supported explorer");
  }
  if (explorer) {
    _runningExplorers.push_back(explorer);
    return explorer;
  }
  throw Exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING,
                  "CosmosLikeWalletFactory using non supported explorer");
}

std::shared_ptr<CosmosLikeBlockchainObserver>
CosmosLikeWalletFactory::getObserver(
    const std::string &currencyName,
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
  auto engine =
      configuration->getString(api::Configuration::BLOCKCHAIN_OBSERVER_ENGINE)
          .value_or(api::CosmosBlockchainObserverEngines::COSMOS_NODE);
  std::shared_ptr<CosmosLikeBlockchainObserver> observer;
  if (engine == api::CosmosBlockchainObserverEngines::COSMOS_NODE) {
    auto ws = services->getWebSocketClient();
    auto context = services->getDispatcher()->getSerialExecutionContext(
        api::CosmosBlockchainObserverEngines::COSMOS_NODE);
    auto logger = services->logger();
    const auto &currency = getCurrency();
    observer = std::make_shared<CosmosLikeBlockchainObserver>(
        context, ws, configuration, logger, currency);
  }
  if (observer)
    _runningObservers.push_back(observer);
  return observer;
}
} // namespace cosmos
} // namespace core
} // namespace ledger
