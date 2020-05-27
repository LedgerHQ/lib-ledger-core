#pragma once

#include <core/api/Account.hpp>
#include <core/api/ErrorCodeCallback.hpp>
#include <core/api/Services.hpp>
#include <core/api/WalletStore.hpp>
#include <tezos/TezosLikeAccount.hpp>
#include <tezos/api/Tezos.hpp>
#include <tezos/operations/TezosLikeOperation.hpp>

namespace ledger {
    namespace core {
        struct Tezos : api::Tezos {
            Tezos() = default;
            ~Tezos() = default;

            void registerInto(
                std::shared_ptr<api::Services> const & services,
                std::shared_ptr<api::WalletStore> const & walletStore,
                std::shared_ptr<api::ErrorCodeCallback> const & callback
            ) override;

            std::shared_ptr<api::TezosLikeAccount> fromCoreAccount(const std::shared_ptr<api::Account> & coreAccount) override;

            std::shared_ptr<api::TezosLikeOperation> fromCoreOperation(const std::shared_ptr<api::Operation> & coreOperation) override;
        };
    }
}

