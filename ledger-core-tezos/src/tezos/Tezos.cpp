#include <core/Services.hpp>
#include <core/api/ErrorCodeCallback.hpp>
#include <core/api/Services.hpp>
#include <core/api/WalletStore.hpp>
#include <core/wallet/WalletStore.hpp>
#include <tezos/TezosLikeCurrencies.hpp>
#include <tezos/api/Tezos.hpp>
#include <tezos/factories/TezosLikeWalletFactory.hpp>

namespace ledger {
    namespace core {
        bool api::Tezos::registerInto(
            std::shared_ptr<api::Services> const & services,
            std::shared_ptr<api::WalletStore> const & walletStore,
        ) {
            auto s = std::dynamic_pointer_cast<ledger::core::Services>(services);
            auto ws = std::dynamic_pointer_cast<ledger::core::WalletStore>(walletStore);
            auto currencies = [
                currencies::tezos(),
            ];

            for (auto const & currency : currencies) {
              wait(ws->addCurrency(currency));
              auto walletFactory = std::make_shared<TezosLikeWalletFactory>(currency, s);
              ws->registerFactory(currency, walletFactory);
            }

            return true;
        }
    }
}

