#pragma once

#include <async/Future.hpp>
#include <vector>
#include <mutex>
#include <wallet/NetworkTypes.hpp>
#include <wallet/BlockchainDatabase.hpp>

namespace ledger {
    namespace core {
        namespace api {
            class ExecutionContext;
        };
        namespace common {
            template<typename NetworkType>
            class InMemoryBlockchainDatabase : public BlockchainDatabase<NetworkType> {
            public:
                typedef typename NetworkType::FilledBlock FilledBlock;
                typedef typename NetworkType::Block Block;

                InMemoryBlockchainDatabase(
                    const std::shared_ptr<api::ExecutionContext>& context);

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
                std::map<uint32_t, FilledBlock> _blocks;
                std::mutex _lock;
            };
        }
    };
}

extern template ledger::core::common::InMemoryBlockchainDatabase<ledger::core::BitcoinLikeNetwork>;
extern template std::shared_ptr<ledger::core::common::InMemoryBlockchainDatabase<ledger::core::BitcoinLikeNetwork>>;