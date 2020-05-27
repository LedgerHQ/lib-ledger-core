#include <core/Services.hpp>
#include <core/api/ErrorCodeCallback.hpp>
#include <core/api/Services.hpp>
#include <core/api/WalletStore.hpp>
#include <core/wallet/WalletStore.hpp>
#include <tezos/Tezos.hpp>
#include <tezos/TezosLikeCurrencies.hpp>
#include <tezos/factories/TezosLikeWalletFactory.hpp>

namespace ledger {
    namespace core {
        std::shared_ptr<api::Tezos> api::Tezos::newInstance() {
            return std::make_shared<ledger::core::Tezos>();
        }

        void Tezos::registerInto(
            std::shared_ptr<api::Services> const & services,
            std::shared_ptr<api::WalletStore> const & walletStore,
            std::shared_ptr<api::ErrorCodeCallback> const & callback
        ) {
            auto currency = currencies::tezos();
            auto s = std::dynamic_pointer_cast<ledger::core::Services>(services);
            auto ws = std::dynamic_pointer_cast<ledger::core::WalletStore>(walletStore);
            auto walletFactory = std::make_shared<TezosLikeWalletFactory>(currency, s);

            ws->addCurrency(currency)
                .template map<api::ErrorCode>(s->getDispatcher()->getMainExecutionContext(), [=] (Unit const & unit) {
                    ws->registerFactory(currency, walletFactory);
                    return api::ErrorCode::FUTURE_WAS_SUCCESSFULL;
                }).callback(s->getDispatcher()->getMainExecutionContext(), callback);
        }

        std::shared_ptr<api::TezosLikeAccount> Tezos::fromCoreAccount(const std::shared_ptr<api::Account> & coreAccount) {
            return std::dynamic_pointer_cast<ledger::core::TezosLikeAccount>(coreAccount);
        }

        std::shared_ptr<api::TezosLikeOperation> Tezos::fromCoreOperation(const std::shared_ptr<api::Operation> & coreOperation) {
            return std::dynamic_pointer_cast<ledger::core::TezosLikeOperation>(coreOperation);
        }
    }
}

