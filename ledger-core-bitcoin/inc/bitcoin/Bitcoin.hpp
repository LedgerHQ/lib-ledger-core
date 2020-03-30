#pragma once

#include <core/api/Account.hpp>
#include <core/api/ErrorCodeCallback.hpp>
#include <core/api/Services.hpp>
#include <core/api/WalletStore.hpp>
#include <bitcoin/BitcoinLikeAccount.hpp>
#include <bitcoin/api/Bitcoin.hpp>
#include <bitcoin/api/BitcoinLikeAccount.hpp>
#include <bitcoin/operations/BitcoinLikeOperation.hpp>

namespace ledger {
    namespace core {
        struct Bitcoin : api::Bitcoin {
            Bitcoin() = default;
            ~Bitcoin() = default;

            void registerInto(
                std::shared_ptr<api::Services> const & services,
                std::shared_ptr<api::WalletStore> const & walletStore,
                std::shared_ptr<api::ErrorCodeCallback> const & callback
            ) override;

            std::shared_ptr<api::BitcoinLikeAccount> fromCoreAccount(const std::shared_ptr<api::Account> & coreAccount) override;

            std::shared_ptr<api::BitcoinLikeOperation> fromCoreOperation(const std::shared_ptr<api::Operation> & coreOperation) override;
        };
    }
}
