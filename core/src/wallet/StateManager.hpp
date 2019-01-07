#pragma once

#include <mutex>
#include <wallet/BlockchainDatabase.hpp>

namespace ledger {
    namespace core {

        template<typename FilledBlock>
        struct BlockchainState {
            typedef ReadOnlyBlockchainDatabase<FilledBlock> BlocksDb;
            typedef typename FilledBlock::Transaction Transaction;
            BlockchainState(
                const std::shared_ptr<BlocksDb>& stableDB,
                const std::shared_ptr<BlocksDb>& unStableDB,
                const std::vector<Transaction>& pending)
            : stableBlocks(stableBlocks)
            , unstableBlocks(unStableDB)
            , pendingTransactions(pending) {
            }
 
            std::shared_ptr<BlocksDb> stableBlocks;
            std::shared_ptr<BlocksDb> unstableBlocks;
            std::vector<Transaction> pendingTransactions;
        };

        template<typename FilledBlock>
        class StateManager {
        public:
            StateManager(const BlockchainState<FilledBlock>& st)
                : _state (st) {
            }
            BlockchainState<FilledBlock> getState() {
                std::lock_guard<std::mutex> lock(_lock);
                return _state;
            }

            void setState(const BlockchainState<FilledBlock>& st) {
                std::lock_guard<std::mutex> lock(_lock);
                _state = st;
            }
        private:
            std::mutex _lock;
            BlockchainState<FilledBlock> _state;
        };
    }
}