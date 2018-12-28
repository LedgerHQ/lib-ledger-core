#pragma once

#include <async/Future.hpp>
#include <async/FutureUtils.hpp>
#include <api/ExecutionContext.hpp>
#include <api/TrustIndicator.hpp>
#include <api/TrustLevel.hpp>
#include <vector>
#include <memory>
#include <wallet/OperationService.hpp>
#include <wallet/Operation.h>
#include <wallet/BlockchainDatabase.hpp>
#include <wallet/Keychain.hpp>

namespace ledger {
    namespace core {
        namespace common {

            template<typename FilledBlock>
            class OperationServiceOnDB : public OperationService, public std::enable_shared_from_this<OperationServiceOnDB<FilledBlock>> {
            public:
                typedef ReadOnlyBlockchainDatabase<FilledBlock> BlocksDB;
            public:
                OperationServiceOnDB(
                    const std::shared_ptr<api::ExecutionContext>& executionContext,
                    const std::shared_ptr<KeychainRegistry> addressRegistry,
                    const std::shared_ptr<BlocksDB>& stableBlocksDB,
                    const std::shared_ptr<BlocksDB>& unstableBlocksDB, 
                    const std::shared_ptr<BlocksDB>& pendingTransactions)
                    : _executionContext(executionContext)
                    , _addressRegistry(addressRegistry)
                    , _stableBlocksDB(stableBlocksDB)
                    , _unstableBlocksDB(unstableBlocksDB)
                    , _pendingTransactions(pendingTransactions) {
                }

                Future<std::vector<Operation>> getOperations(uint32_t fromBlock, uint32_t toBlock) override {
                    std::vector<Future<std::vector<FilledBlock>>> futures {
                        _unstableBlocksDB->getBlocks(fromBlock, toBlock),
                        _stableBlocksDB->getBlocks(fromBlock, toBlock)
                    };
                    auto self = this->shared_from_this();
                    return executeAll(_executionContext, futures)
                        .template map<std::vector<Operation>>(_executionContext, [self](const std::vector<std::vector<FilledBlock>>& vectorBlocks) {
                            std::vector<Operation> res;
                            self->addOperations(vectorBlocks[0], api::TrustLevel::UNTRUSTED, res);
                            self->addOperations(vectorBlocks[1], api::TrustLevel::TRUSTED, res);
                            return res;
                        });
                }

                Future<std::vector<Operation>> getPendingOperations() override {
                    auto self = this->shared_from_this();
                    return _pendingTransactions->getLastBlock()
                        .template map<std::vector<Operation>>(_executionContext, [self](const Option<std::pair<uint32_t, FilledBlock>>& lastBlock) {
                            std::vector<Operation> res;
                            if (!lastBlock.hasValue())
                                return res;
                            self->addOperation(lastBlock.getValue().second, api::TrustLevel::PENDING, res);
                            return res;
                         });
                }
            private:
                void addOperation(const FilledBlock& block, api::TrustLevel trustLevel, std::vector<Operation>& res) {
                    for (auto& transaction : block.transactions) {
                        Operation op = core::createOperation(transaction, _addressRegistry);
                        op.trust = std::make_shared<TrustIndicator>();
                        op.trust->setTrustLevel(trustLevel);
                        res.push_back(op);
                    }
                }

                void addOperations(const std::vector<FilledBlock>& blocks, api::TrustLevel trustLevel, std::vector<Operation>& res) {
                    for (auto& block : blocks) {
                        addOperation(block, trustLevel, res);
                    }
                }
            private:
                std::shared_ptr<api::ExecutionContext> _executionContext;
                std::shared_ptr<KeychainRegistry> _addressRegistry;
                std::shared_ptr<BlocksDB> _stableBlocksDB;
                std::shared_ptr<BlocksDB> _unstableBlocksDB;
                std::shared_ptr<BlocksDB> _pendingTransactions;
            };
        }
    }
}
