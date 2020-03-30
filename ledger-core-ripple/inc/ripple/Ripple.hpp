#pragma once

#include <core/api/Account.hpp>
#include <core/api/ErrorCodeCallback.hpp>
#include <core/api/Services.hpp>
#include <core/api/WalletStore.hpp>
#include <ripple/RippleLikeAccount.hpp>
#include <ripple/RippleLikeOperation.hpp>
#include <ripple/api/Ripple.hpp>

namespace ledger {
    namespace core {
        struct Ripple : api::Ripple {
            Ripple() = default;
            ~Ripple() = default;

            void registerInto(
                std::shared_ptr<api::Services> const & services,
                std::shared_ptr<api::WalletStore> const & walletStore,
                std::shared_ptr<api::ErrorCodeCallback> const & callback
            ) override;

            std::shared_ptr<api::RippleLikeAccount> fromCoreAccount(const std::shared_ptr<api::Account> & coreAccount) override;

            std::shared_ptr<api::RippleLikeOperation> fromCoreOperation(const std::shared_ptr<api::Operation> & coreOperation) override;
        };
    }
}

