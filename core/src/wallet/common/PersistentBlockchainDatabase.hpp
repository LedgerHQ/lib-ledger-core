#pragma once

#include <async/Future.hpp>
#include <vector>
#include <wallet/NetworkTypes.hpp>
#include <wallet/BlockchainDatabase.hpp>

namespace ledger {
    namespace core {
        namespace api {
            class ExecutionContext;
        };
        namespace db {
            class BlockchainDB;
        };
        namespace common {
            template<typename NetworkType>
            class PersistentBlockchainDatabase : public BlockchainDatabase<NetworkType> {
            public:
                typedef typename NetworkType::FilledBlock FilledBlock;
                typedef typename NetworkType::Block Block;

                PersistentBlockchainDatabase(
                    const std::shared_ptr<api::ExecutionContext>& context,
                    const std::shared_ptr<db::BlockchainDB>& persistentDB);

                Future<std::vector<FilledBlock>> getBlocks(uint32_t heightFrom, uint32_t heightTo) override;

                Future<Option<FilledBlock>> getBlock(uint32_t height) override;
                // Return the last block header or genesis block
                Future<Option<Block>> getLastBlockHeader() override;
                void addBlock(const FilledBlock& block) override;
                void removeBlocks(uint32_t heightFrom, uint32_t heightTo) override;
                void removeBlocksUpTo(uint32_t heightTo) override;
                void CleanAll() override;
            private:
                std::shared_ptr<api::ExecutionContext> _context;
                std::shared_ptr<db::BlockchainDB> _persistentDB;
            };
        }
    };
}

extern template ledger::core::common::PersistentBlockchainDatabase<ledger::core::BitcoinLikeNetwork>;
extern template std::shared_ptr<ledger::core::common::PersistentBlockchainDatabase<ledger::core::BitcoinLikeNetwork>>;