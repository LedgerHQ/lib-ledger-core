#pragma once

#include <memory>

#include <core/Services.hpp>
#include <core/wallet/WalletStore.hpp>
#include <core/api/Currency.hpp>
#include <core/api/ErrorCode.hpp>
#include <core/async/Future.hpp>
#include <core/utils/Unit.hpp>

namespace ledger {
    namespace core {
        /// A helper function used to register a list of currencies into a services / wallet store.
        template <typename CoinFactory>
        ledger::core::Future<ledger::core::api::ErrorCode> registerCurrenciesInto(
            std::shared_ptr<ledger::core::Services> const & services,
            std::shared_ptr<ledger::core::WalletStore> const & walletStore,
            std::shared_ptr<std::vector<ledger::core::api::Currency>> const & currencies
        ) {
            if (currencies->empty()) {
                return ledger::core::Future<ledger::core::api::ErrorCode>::successful(ledger::core::api::ErrorCode::FUTURE_WAS_SUCCESSFULL);
            }

            // get a currency and decrease the size of the currencies to to treat
            auto currency = currencies->back();
            currencies->pop_back();

            return walletStore->addCurrency(currency)
                .template flatMap<ledger::core::api::ErrorCode>(services->getDispatcher()->getMainExecutionContext(), [=] (ledger::core::Unit const & unit) {
                    auto walletFactory = std::make_shared<CoinFactory>(currency, services);
                    walletStore->registerFactory(currency, walletFactory);
                    return registerCurrenciesInto<CoinFactory>(services, walletStore, currencies);
                });
        }
    }
}
