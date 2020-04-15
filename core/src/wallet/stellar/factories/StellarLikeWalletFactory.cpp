/*
 *
 * StellarLikeWalletFactory.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 21/02/2019.
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

#include "StellarLikeWalletFactory.hpp"
#include <wallet/stellar/StellarLikeWallet.hpp>
#include "StellarLikeKeychainFactory.hpp"
#include <wallet/pool/WalletPool.hpp>
#include <wallet/stellar/explorers/HorizonBlockchainExplorer.hpp>
#include <api/StellarConfiguration.hpp>
#include <wallet/stellar/synchronizers/StellarLikeBlockchainExplorerAccountSynchronizer.hpp>

#define STRING(key, def) entry.configuration->getString(key).value_or(def)

namespace ledger {
    namespace core {

        StellarLikeWalletFactory::StellarLikeWalletFactory(const api::Currency &currency,
                                                           const std::shared_ptr<WalletPool> &pool)
                : AbstractWalletFactory(currency, pool) {

        }

        std::shared_ptr<AbstractWallet> StellarLikeWalletFactory::build(const WalletDatabaseEntry &entry) {
            StellarLikeWalletParams params;
            auto currency = getPool()->getCurrency(entry.currencyName).getOrElse([&] () -> api::Currency {
                throw make_exception(api::ErrorCode::CURRENCY_NOT_FOUND, "Currency '{}' not found", entry.currencyName);
            });

            // Get derivation scheme
            DerivationScheme scheme(STRING(api::Configuration::KEYCHAIN_DERIVATION_SCHEME, "44'/<coin_type>'/<account>'"));

            // Configure keychain factory
            params.keychainFactory = std::make_shared<StellarLikeKeychainFactory>();

            // Configure explorer
            params.blockchainExplorer = getExplorer(entry);
            // Configure observer

            // Configure synchronizer
            // This line will need to be changed if we have multiple synchronizers for now it's ok.
            params.accountSynchronizer = std::make_shared<StellarLikeBlockchainExplorerAccountSynchronizer>(getPool(), params.blockchainExplorer);

            return std::make_shared<StellarLikeWallet>(entry.name, currency, getPool(), entry.configuration, scheme, params);
        }

        template <>
        std::shared_ptr<AbstractWalletFactory> make_factory<api::WalletType::STELLAR>(const api::Currency& currency,
                const std::shared_ptr<WalletPool>& pool) {
            return std::make_shared<StellarLikeWalletFactory>(currency, pool);
        }

        std::shared_ptr<StellarLikeBlockchainExplorer> StellarLikeWalletFactory::getExplorer(const WalletDatabaseEntry& entry) {
            auto engine = STRING(api::Configuration::BLOCKCHAIN_EXPLORER_ENGINE, api::StellarConfiguration::HORIZON_EXPLORER_ENGINE);
            std::shared_ptr<StellarLikeBlockchainExplorer> explorer;
            if (engine == api::StellarConfiguration::HORIZON_EXPLORER_ENGINE) {
                auto baseUrl = STRING(api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT, api::StellarConfiguration::HORIZON_MAINNET_BLOCKCHAIN_EXPLORER_URL);
                explorer = std::make_shared<HorizonBlockchainExplorer>(getPool()->getDispatcher()->getSerialExecutionContext("stellar_explorer"), getPool()->getHttpClient(baseUrl), entry.configuration);
            }
            return explorer;
        }

        inline std::shared_ptr<api::ExecutionContext> StellarLikeWalletFactory::getContext(const WalletDatabaseEntry& entry) {
            return getPool()->getDispatcher()->getSerialExecutionContext(fmt::format("stellar:{}", entry.name));
        }

    }
}
