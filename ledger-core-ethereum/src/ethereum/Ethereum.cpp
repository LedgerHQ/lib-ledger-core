#include <core/Services.hpp>
#include <core/api/ErrorCodeCallback.hpp>
#include <core/api/Services.hpp>
#include <core/api/WalletStore.hpp>
#include <core/wallet/WalletStore.hpp>
#include <ethereum/Ethereum.hpp>
#include <ethereum/EthereumLikeCurrencies.hpp>
#include <ethereum/factories/EthereumLikeWalletFactory.hpp>

namespace {
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
                auto walletFactory = std::make_shared<ledger::core::EthereumLikeWalletFactory>(currency, services);
                walletStore->registerFactory(currency, walletFactory);
                return registerCurrenciesInto(services, walletStore, currencies);
            });
    }
}

namespace ledger {
    namespace core {
        std::shared_ptr<api::Ethereum> api::Ethereum::newInstance() {
          return std::make_shared<ledger::core::Ethereum>();
        }

        void Ethereum::registerInto(
            std::shared_ptr<api::Services> const & services,
            std::shared_ptr<api::WalletStore> const & walletStore,
            std::shared_ptr<api::ErrorCodeCallback> const & callback
        ) {
            auto s = std::dynamic_pointer_cast<ledger::core::Services>(services);
            auto ws = std::dynamic_pointer_cast<ledger::core::WalletStore>(walletStore);
            auto currencies = std::make_shared<std::vector<api::Currency>>(std::vector<api::Currency>{
                currencies::ethereum(),
                currencies::ethereum_classic(),
                currencies::ethereum_ropsten()
            });

            registerCurrenciesInto(s, ws, currencies)
              .callback(s->getDispatcher()->getMainExecutionContext(), callback);
        }

        std::shared_ptr<api::EthereumLikeAccount> Ethereum::fromCoreAccount(const std::shared_ptr<api::Account> & coreAccount) {
          return std::dynamic_pointer_cast<ledger::core::EthereumLikeAccount>(coreAccount);
        }

        std::shared_ptr<api::EthereumLikeOperation> Ethereum::fromCoreOperation(const std::shared_ptr<api::Operation> & coreOperation) {
          return std::dynamic_pointer_cast<ledger::core::EthereumLikeOperation>(coreOperation);
        }
    }
}

