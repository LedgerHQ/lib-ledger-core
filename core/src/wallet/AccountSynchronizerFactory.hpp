#pragma once

#include <memory>
#include <wallet/AccountSynchronizer.hpp>
#include <wallet/BlockchainDatabase.hpp>

namespace ledger {
    namespace core {
        template<typename NetworkType>
        class AccountSynchronizerFactory {
        public:
            virtual std::shared_ptr<AccountSynchronizer> createAccountSynchronizer(
                std::shared_ptr<BlockchainDatabase<NetworkType>> stableBlocksDb,
                std::shared_ptr<BlockchainDatabase<NetworkType>> unstableBlocksDb) = 0;
            virtual ~AccountSynchronizerFactory() {};
        };
    }
}
