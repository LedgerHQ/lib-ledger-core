#include <core/Services.hpp>
#include <core/api/ErrorCodeCallback.hpp>
#include <core/api/Services.hpp>
#include <core/api/WalletStore.hpp>
#include <core/wallet/WalletStore.hpp>
#include <bitcoin/BitcoinLikeCurrencies.hpp>
#include <bitcoin/api/Bitcoin.hpp>
#include <bitcoin/factories/BitcoinLikeWalletFactory.hpp>

namespace ledger {
    namespace core {
        bool api::Bitcoin::registerInto(
            std::shared_ptr<api::Services> const & services,
            std::shared_ptr<api::WalletStore> const & walletStore,
        ) {
            auto s = std::dynamic_pointer_cast<ledger::core::Services>(services);
            auto ws = std::dynamic_pointer_cast<ledger::core::WalletStore>(walletStore);
            auto currencies = [
                currencies::bitcoin(),
                currencies::bitcoin_testnet(),
                currencies::bitcoin_cash(),
                currencies::bitcoin_gold(),
                currencies::zcash(),
                currencies::zencash(),
                currencies::litecoin(),
                currencies::peercoin(),
                currencies::digibyte(),
                currencies::hcash(),
                currencies::qtum(),
                currencies::stealthcoin(),
                currencies::vertcoin(),
                currencies::viacoin(),
                currencies::dash(),
                currencies::dogecoin(),
                currencies::stratis(),
                currencies::komodo(),
                currencies::poswallet(),
                currencies::pivx(),
                currencies::clubcoin(),
                currencies::decred(),
                currencies::stakenet()
            ];

            for (auto const & currency : currencies) {
              wait(ws->addCurrency(currency));
              auto walletFactory = std::make_shared<BitcoinLikeWalletFactory>(currency, s);
              ws->registerFactory(currency, walletFactory);
            }

            return true;
        }
    }
}
