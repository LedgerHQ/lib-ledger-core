#pragma once

#include <wallet/BlochainDatabase.hpp>
#include <vector>

namespace ledger {
    namespace core {
        template<typename NetworkType>
        class BlockchainDatabaseFactory {
        public:
            typedef NetworkType::AccountDescriptor AccountDescriptor;
            virtual ~BlockchainDatabaseFactory() = 0;
            virtual std::shared_ptr<BlockchainDatabase<NetworkType>> CreateStableBlocksDB(const std::shared_ptr<AccountDescriptor>& descriptor) = 0;
            virtual std::shared_ptr<BlockchainDatabase<NetworkType>> CreateUnstableBlocksDB(const std::shared_ptr<AccountDescriptor>& descriptor) = 0;
        };
    };
}