#include <core/Services.hpp>
#include <core/api/ErrorCodeCallback.hpp>
#include <core/api/Services.hpp>
#include <core/api/WalletStore.hpp>
#include <core/utils/Coin.hpp>
#include <core/wallet/WalletStore.hpp>
#include <bitcoin/Bitcoin.hpp>
#include <bitcoin/BitcoinLikeCurrencies.hpp>
#include <bitcoin/factories/BitcoinLikeWalletFactory.hpp>

namespace ledger {
    namespace core {
        std::shared_ptr<api::Bitcoin> api::Bitcoin::newInstance() {
          return std::make_shared<ledger::core::Bitcoin>();
        }

        void Bitcoin::registerInto(
            std::shared_ptr<api::Services> const & services,
            std::shared_ptr<api::WalletStore> const & walletStore,
            std::shared_ptr<api::ErrorCodeCallback> const & callback
        ) {
            auto s = std::dynamic_pointer_cast<ledger::core::Services>(services);
            auto ws = std::dynamic_pointer_cast<ledger::core::WalletStore>(walletStore);
            auto currencies = std::make_shared<std::vector<api::Currency>>(std::vector<api::Currency>{
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
            });

            registerCurrenciesInto<ledger::core::BitcoinLikeWalletFactory>(s, ws, currencies)
              .callback(s->getDispatcher()->getMainExecutionContext(), callback);
        }

        std::shared_ptr<api::BitcoinLikeAccount> Bitcoin::fromCoreAccount(const std::shared_ptr<api::Account> & coreAccount) {
            return std::dynamic_pointer_cast<ledger::core::BitcoinLikeAccount>(coreAccount);
        }

        std::shared_ptr<api::BitcoinLikeOperation> Bitcoin::fromCoreOperation(const std::shared_ptr<api::Operation> & coreOperation) {
            return std::dynamic_pointer_cast<ledger::core::BitcoinLikeOperation>(coreOperation);
        }
    }
}
