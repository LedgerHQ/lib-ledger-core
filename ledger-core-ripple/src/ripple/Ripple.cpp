#include <core/Services.hpp>
#include <core/api/ErrorCodeCallback.hpp>
#include <core/api/Services.hpp>
#include <core/api/WalletStore.hpp>
#include <core/wallet/WalletStore.hpp>
#include <ripple/Ripple.hpp>
#include <ripple/RippleLikeCurrencies.hpp>
#include <ripple/factories/RippleLikeWalletFactory.hpp>

namespace ledger {
    namespace core {
        std::shared_ptr<api::Ripple> api::Ripple::newInstance() {
          return std::make_shared<ledger::core::Ripple>();
        }

        void Ripple::registerInto(
            std::shared_ptr<api::Services> const & services,
            std::shared_ptr<api::WalletStore> const & walletStore,
            std::shared_ptr<api::ErrorCodeCallback> const & callback
        ) {
            auto currency = currencies::ripple();
            auto s = std::dynamic_pointer_cast<ledger::core::Services>(services);
            auto ws = std::dynamic_pointer_cast<ledger::core::WalletStore>(walletStore);
            auto walletFactory = std::make_shared<RippleLikeWalletFactory>(currency, s);

            ws->addCurrency(currency)
                .template map<api::ErrorCode>(s->getDispatcher()->getMainExecutionContext(), [=] (Unit const & unit) {
                    ws->registerFactory(currency, walletFactory);
                    return api::ErrorCode::FUTURE_WAS_SUCCESSFULL;
                }).callback(s->getDispatcher()->getMainExecutionContext(), callback);
        }

        std::shared_ptr<api::RippleLikeAccount> Ripple::fromCoreAccount(
            const std::shared_ptr<api::Account> & coreAccount
        ) {
          return std::dynamic_pointer_cast<ledger::core::RippleLikeAccount>(coreAccount);
        }

        std::shared_ptr<api::RippleLikeOperation> Ripple::fromCoreOperation(const std::shared_ptr<api::Operation> & coreOperation) {
          return std::dynamic_pointer_cast<api::RippleLikeOperation>(coreOperation);
        }
    }
}
