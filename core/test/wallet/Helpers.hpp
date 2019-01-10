#pragma once
#include <async/Future.hpp>
#include <vector>
#include <map>
#include <queue>
#include <boost/lexical_cast.hpp>
#include <wallet/NetworkTypes.hpp>
#include <wallet/bitcoin/UTXO.hpp>
#include <gtest/gtest.h>

namespace ledger {
    namespace core {
        namespace bitcoin {
            bool operator==(const Block& lhs, const Block& rhs);
            bool operator==(const Transaction& lhs, const Transaction& rhs);
            bool operator==(const Input& lhs, const Input& rhs);
            bool operator==(const Output& lhs, const Output& rhs);
            bool operator==(const UTXOValue& lhs, const UTXOValue& rhs);
        }

        bool operator==(const BitcoinLikeNetwork::FilledBlock& lhs, const BitcoinLikeNetwork::FilledBlock& rhs);

        namespace tests {
            template<typename T>
            T getFutureResult(const Future<T>& f) {
                EXPECT_TRUE(f.isCompleted());
                EXPECT_TRUE(f.getValue().hasValue());
                EXPECT_TRUE(f.getValue().getValue().isSuccess());
                return f.getValue().getValue().getValue();
            }

            template<typename T>
            T getOptionFutureResult(const Future<Option<T>>& f) {
                Option<T> o = getFutureResult(f);
                EXPECT_TRUE(o.hasValue());
                return o.getValue();
            }

            template<typename T>
            bool checkOptionFutureHasNoValue(const Future<Option<T>>& f) {
                Option<T> o = getFutureResult(f);
                EXPECT_TRUE(o.isEmpty());
            }

            class SimpleExecutionContext : public api::ExecutionContext {
            public:
                virtual void execute(const std::shared_ptr<api::Runnable> & runnable) override {
                    q.push(runnable);
                }

                void delay(const std::shared_ptr<api::Runnable> & runnable, int64_t millis) override {
                    throw "notImplemented";
                }

                bool runOne() {
                    if (q.empty())
                        return false;
                    auto x = q.front();
                    q.pop();
                    x->run();
                    return true;
                };

                void wait() {
                    while (runOne());
                };
            private:
                std::queue<std::shared_ptr<api::Runnable>> q;
            };

            //Transaction
            struct TR {
                std::vector<std::string> inputs;
                /// The first part of the pair is the address and the second part is the amount.
                std::vector<std::pair<std::string, uint32_t>> outputs;
            };

            //Block
            struct BL {
                uint32_t height;
                std::string hash;
                std::vector<TR> transactions;
            };

            BitcoinLikeNetwork::Block toBlock(const BL& b);
            BitcoinLikeNetwork::Transaction toTran(const BL& b, const TR& tr);
            std::vector<BitcoinLikeNetwork::FilledBlock> toFilledBlocks(const std::vector<BL>& blocks);
            BitcoinLikeNetwork::FilledBlock toFilledBlock(const BL& block);

            class FakeExplorer {
            public:
                typedef BitcoinLikeNetwork::Transaction Tran;
                typedef BitcoinLikeNetwork::Block Block;
                typedef std::pair<std::vector<Tran>, bool> TransactionBulk;
                typedef BitcoinLikeNetwork::Input Input;
                typedef BitcoinLikeNetwork::Output Output;

                FakeExplorer() : _numberOfTransactionsAllowed(200) {};

                void setBlockchain(const std::vector<BitcoinLikeNetwork::FilledBlock>& blockchain);

                void setTruncationLevel(uint32_t numberOfTransactionsAllowed);

                Future<TransactionBulk> getTransactions(const std::vector<std::string>& addresses, Option<std::string> fromBlockHash, Option<void*> session); 
                Future<Block> getCurrentBlock();
            private:
                std::vector<Tran> _transactions;
                std::map<std::string, uint32_t> _blockHashes;
                uint32_t _numberOfTransactionsAllowed;
            };

            class FakeKeyChain {
            public:
                FakeKeyChain(uint32_t alreadyUsed, uint32_t seed);

                uint32_t getNumberOfUsedAddresses();

                std::vector<std::string> getAddresses(uint32_t startIndex, uint32_t count);

                void markAsUsed(const std::string& address);

            private:
                uint32_t _alreadyUsed;
                uint32_t _seed;
            };

            bool TransEqual(const BitcoinLikeNetwork::Transaction& tran, const TR& tr);
            std::function<bool(const BitcoinLikeNetwork::FilledBlock&)> Same(const BL& left);
        }
    }
}
