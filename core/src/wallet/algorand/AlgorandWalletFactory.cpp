/*
 *
 * AlgorandWalletFactory
 *
 * Created by Hakim Aammar on 11/05/2020.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Ledger
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

#include "AlgorandWalletFactory.hpp"
#include "AlgorandWallet.hpp"
#include "AlgorandNetworks.hpp"

#include <utils/Assert.hpp>

#include <api/AlgorandBlockchainExplorerEngines.hpp>
#include <api/Configuration.hpp>
#include <api/SynchronizationEngines.hpp>

namespace ledger {
namespace core {
namespace algorand {

    // Aliases for long constants names
    const std::string ALGORAND_API_ENDPOINT = "https://mainnet-algorand.api.purestake.io";
    const std::string ALGORAND_NODE_EXPLORER = api::AlgorandBlockchainExplorerEngines::ALGORAND_NODE;

    WalletFactory::WalletFactory(const api::Currency &currency, const std::shared_ptr<WalletPool>& pool) :
        AbstractWalletFactory(currency, pool)
    {}

    std::shared_ptr<AbstractWallet> WalletFactory::build(const WalletDatabaseEntry &entry) {
        auto pool = getPool();
        pool->logger()->info("Building wallet instance '{}' for {} with parameters: {}",
            entry.name,
            entry.currencyName,
            entry.configuration->dump());

        // Retrieve the API key from env variable, if set
        if (const auto apiKey = std::getenv(api::Configuration::BLOCKCHAIN_EXPLORER_API_KEY.c_str())) {
            entry.configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_API_KEY, apiKey);
        }

        // Make sure the currency is Algorand-like
        assertCurrency(
            networks::isAlgorandCurrency(entry.currencyName),
            fmt::format("Currency '{}' is not an Algorand currency.", entry.currencyName)
        );

        // Configure explorer
        auto explorer = getExplorer(entry.currencyName, entry.configuration);
        if (explorer == nullptr) {
            throw make_exception(api::ErrorCode::UNKNOWN_BLOCKCHAIN_EXPLORER_ENGINE,
                                 "Engine '{}' is not a supported explorer engine.",
                                 entry.configuration->getString(api::Configuration::BLOCKCHAIN_EXPLORER_ENGINE).value_or("undefined"));
        }

        // Configure synchronizer
        auto syncEngine = entry.configuration->getString(api::Configuration::SYNCHRONIZATION_ENGINE)
                                               .value_or(api::SynchronizationEngines::BLOCKCHAIN_EXPLORER_SYNCHRONIZATION);
        Option<AlgorandAccountSynchronizerFactory> synchronizerFactory;
        {
            if (syncEngine == api::SynchronizationEngines::BLOCKCHAIN_EXPLORER_SYNCHRONIZATION) {
                std::weak_ptr<WalletPool> weakPool = pool;
                synchronizerFactory = Option<AlgorandAccountSynchronizerFactory>([weakPool, explorer]() {
                    auto pool = weakPool.lock();
                    if (!pool) {
                        throw make_exception(api::ErrorCode::NULL_POINTER, "Pool was released.");
                    }
                    return std::make_shared<AccountSynchronizer>(pool, explorer);
                });
            }
        }

        if (synchronizerFactory.isEmpty()) {
            throw make_exception(api::ErrorCode::UNKNOWN_SYNCHRONIZATION_ENGINE,
                                "Engine '{}' is not a supported synchronization engine.",
                                syncEngine);
        }

        // Sets the derivation scheme
        DerivationScheme scheme(entry.configuration->getString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME)
                                                    .value_or("44'/<coin_type>'/<account>'/<node>/<address>"));

        // Build wallet
        return std::make_shared<Wallet>(
            entry.name,
            getCurrency(),
            pool,
            entry.configuration,
            scheme,
            explorer,
            synchronizerFactory.getValue()
        );
    }

    std::shared_ptr<BlockchainExplorer>
    WalletFactory::getExplorer(const std::string &currencyName, const std::shared_ptr<api::DynamicObject> &configuration) {

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

        std::shared_ptr<BlockchainExplorer> explorer = nullptr;

        auto engine = configuration->getString(api::Configuration::BLOCKCHAIN_EXPLORER_ENGINE).value_or(ALGORAND_NODE_EXPLORER);
        if (engine == ALGORAND_NODE_EXPLORER) {
            auto &networkParams = networks::getAlgorandNetworkParameters(getCurrency().name);
            auto pool = getPool();
            auto http = pool->getHttpClient(
                configuration->getString(api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT).value_or(ALGORAND_API_ENDPOINT)
            );
            auto context = pool->getDispatcher()->getSerialExecutionContext(
                    fmt::format("{}-{}-explorer", ALGORAND_NODE_EXPLORER, networkParams.genesisHash)
            );

            explorer = std::make_shared<BlockchainExplorer>(context,
                                                            http,
                                                            networkParams,
                                                            configuration);
        }

        if (explorer) {
            _runningExplorers.push_back(explorer);
        }

        return explorer;
    }

}
}
}
