#include <core/Services.hpp>
#include <core/api/ErrorCodeCallback.hpp>
#include <core/api/Services.hpp>
#include <core/api/WalletStore.hpp>
#include <core/wallet/WalletStore.hpp>
#include <ethereum/EthereumLikeCurrencies.hpp>
#include <ethereum/api/Ethereum.hpp>
#include <ethereum/factories/EthereumLikeWalletFactory.hpp>

namespace ledger {
    namespace core {
        bool api::Ethereum::registerInto(
            std::shared_ptr<api::Services> const & services,
            std::shared_ptr<api::WalletStore> const & walletStore,
        ) {
            auto s = std::dynamic_pointer_cast<ledger::core::Services>(services);
            auto ws = std::dynamic_pointer_cast<ledger::core::WalletStore>(walletStore);
            auto currencies = [
                currencies::ethereum(),
                currencies::ethereum_classic(),
                currencies::ethereum_ropsten()
            ];

            for (auto const & currency : currencies) {
              wait(ws->addCurrency(currency));
              auto walletFactory = std::make_shared<EthereumLikeWalletFactory>(currency, s);
              ws->registerFactory(currency, walletFactory);
            }

            return true;
        }
    }
}

