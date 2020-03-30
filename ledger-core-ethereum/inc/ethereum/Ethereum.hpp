#pragma once

#include <core/api/Account.hpp>
#include <core/api/ErrorCodeCallback.hpp>
#include <core/api/Services.hpp>
#include <core/api/WalletStore.hpp>
#include <ethereum/EthereumLikeAccount.hpp>
#include <ethereum/api/Ethereum.hpp>
#include <ethereum/operations/EthereumLikeOperation.hpp>

namespace ledger {
    namespace core {
        struct Ethereum : api::Ethereum {
            Ethereum() = default;
            ~Ethereum() = default;

            void registerInto(
                std::shared_ptr<api::Services> const & services,
                std::shared_ptr<api::WalletStore> const & walletStore,
                std::shared_ptr<api::ErrorCodeCallback> const & callback
            ) override;

            std::shared_ptr<api::EthereumLikeAccount> fromCoreAccount(const std::shared_ptr<api::Account> & coreAccount) override;

            std::shared_ptr<api::EthereumLikeOperation> fromCoreOperation(const std::shared_ptr<api::Operation> & coreOperation) override;
        };
    }
}
