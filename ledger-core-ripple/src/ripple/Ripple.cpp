#include <core/Services.hpp>
#include <core/api/ErrorCodeCallback.hpp>
#include <core/api/Services.hpp>
#include <core/api/WalletStore.hpp>
#include <core/wallet/WalletStore.hpp>
#include <ripple/RippleLikeCurrencies.hpp>
#include <ripple/api/Ripple.hpp>
#include <ripple/factories/RippleLikeWalletFactory.hpp>

namespace ledger {
    namespace core {
        bool api::Ripple::registerInto(
            std::shared_ptr<api::Services> const & services,
            std::shared_ptr<api::WalletStore> const & walletStore,
        ) {
            auto s = std::dynamic_pointer_cast<ledger::core::Services>(services);
            auto ws = std::dynamic_pointer_cast<ledger::core::WalletStore>(walletStore);
            auto currencies = [
                currencies::ripple(),
            ];

            for (auto const & currency : currencies) {
              wait(ws->addCurrency(currency));
              auto walletFactory = std::make_shared<RippleLikeWalletFactory>(currency, s);
              ws->registerFactory(currency, walletFactory);
            }

            return true;
        }
    }
}
