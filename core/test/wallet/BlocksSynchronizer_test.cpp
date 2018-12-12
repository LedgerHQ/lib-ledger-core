#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <wallet/NetworkTypes.hpp>
#include <wallet/common/BlocksSynchronizer.hpp>
#include <wallet/common/InMemoryPartialBlocksDB.hpp>
#include <vector>

#include "Helpers.hpp"
#include "Mocks.hpp"

using namespace testing;

namespace ledger {
    namespace core {
        namespace tests {

            class BlockSyncTest : public ::testing::Test {
            public:
                BlockSyncTest()
                    : receivedFake(0, 0) // receive addresses are 0,1,2... change are 1000, 1001...
                    , changeFake(0, 1000) {
                    explorerMock = std::make_shared<NiceMock<ExplorerMock>>();
                    keychainReceiveMock = std::make_shared<NiceMock<KeychainMock>>();
                    keychainChangeMock = std::make_shared<NiceMock<KeychainMock>>();
                    blocksDBMock = std::make_shared<NiceMock<BlocksDBMock>>();
                    context = std::make_shared<SimpleExecutionContext>();
                    
                }

                void SetUp(uint32_t batchSize, uint32_t gapSize, uint32_t maxTransactionsPerResponse = 200) {
                    synchronizer = std::make_shared<common::BlocksSynchronizer<BitcoinLikeNetwork>>(
                        context,
                        explorerMock,
                        keychainReceiveMock,
                        keychainChangeMock,
                        blocksDBMock,
                        gapSize,
                        batchSize,
                        maxTransactionsPerResponse);
                }

                void setupFakeKeychains() {
                    ON_CALL(*keychainReceiveMock, getNumberOfUsedAddresses()).WillByDefault(Invoke(&receivedFake, &FakeKeyChain::getNumberOfUsedAddresses));
                    ON_CALL(*keychainReceiveMock, getAddresses(_, _)).WillByDefault(Invoke(&receivedFake, &FakeKeyChain::getAddresses));
                    ON_CALL(*keychainReceiveMock, markAsUsed(_)).WillByDefault(Invoke(&receivedFake, &FakeKeyChain::markAsUsed));
                    ON_CALL(*keychainChangeMock, getNumberOfUsedAddresses()).WillByDefault(Invoke(&changeFake, &FakeKeyChain::getNumberOfUsedAddresses));
                    ON_CALL(*keychainChangeMock, getAddresses(_, _)).WillByDefault(Invoke(&changeFake, &FakeKeyChain::getAddresses));
                    ON_CALL(*keychainChangeMock, markAsUsed(_)).WillByDefault(Invoke(&changeFake, &FakeKeyChain::markAsUsed));
                };

                void setBlockchain(const std::vector<BL>& blockChain) {
                    fakeExplorer.setBlockchain(toFilledBlocks(blockChain));
                    ON_CALL(*explorerMock, getTransactions(_, _, _)).WillByDefault(Invoke(&fakeExplorer, &FakeExplorer::getTransactions));
                }

                bool equal(const BitcoinLikeNetwork::Transaction& tran, const TR& tr) {
                    if ((tran.inputs.size() != tr.inputs.size()) ||
                        (tran.outputs.size() != tr.outputs.size()))
                        return false;
                    for (int j = 0; j < tran.inputs.size(); ++j) {
                        if (tran.inputs[j].address != tr.inputs[j])
                            return false;
                    }
                    for (int j = 0; j < tran.outputs.size(); ++j) {
                        if (tran.outputs[j].address != tr.outputs[j].first)
                            return false;
                    }
                    return true;
                }

                std::function<bool(const BitcoinLikeNetwork::FilledBlock& )> Same(const BL& left) {
                     return [left](const BitcoinLikeNetwork::FilledBlock& right) 
                     { 
                         if ((left.hash != right.header.hash) ||
                             (left.height != right.header.height) ||
                             (left.transactions.size() != right.transactions.size()))
                             return false;
                         for (int i = 0; i < left.transactions.size(); ++i) {
                             
                         }
                         return true;
                     };
                };
            public:
                std::shared_ptr<common::BlocksSynchronizer<BitcoinLikeNetwork>> synchronizer;
                std::shared_ptr<NiceMock<ExplorerMock>> explorerMock;
                std::shared_ptr<NiceMock<KeychainMock>> keychainReceiveMock;
                std::shared_ptr<NiceMock<KeychainMock>> keychainChangeMock;
                std::shared_ptr<NiceMock<BlocksDBMock>> blocksDBMock;
                std::shared_ptr<SimpleExecutionContext> context;
                FakeKeyChain receivedFake;
                FakeKeyChain changeFake;
                FakeExplorer fakeExplorer;
            };

            TEST_F(BlockSyncTest, OneTransaction) {
                SetUp(1, 1);
                setupFakeKeychains();
                std::vector<BL> bch =
                {
                    BL{ 1, "block 1",
                    {
                        TR{ { "X" },{ { "0", 10000 } } }
                    }}
                };
                setBlockchain(bch);
                EXPECT_CALL(*blocksDBMock, addBlock(Truly(Same(bch[0]))));
                auto f = synchronizer->synchronize("block 1", 1, 10);
                context->wait();
                ASSERT_TRUE(f.isCompleted());
                EXPECT_TRUE(f.getValue().getValue().isSuccess());
            }

            TEST_F(BlockSyncTest, TwoTransOneBlock) {
                SetUp(1, 1);
                setupFakeKeychains();
                std::vector<BL> bch =
                {
                    BL{ 1, "block 1",
                    {
                        TR{ { "X" },{ { "0", 10000 } } },
                        TR{ { "Y" },{ { "0", 10000 } } }
                    } }
                };
                setBlockchain(bch);
                EXPECT_CALL(*blocksDBMock, addBlock(Truly(Same(bch[0]))));
                auto f = synchronizer->synchronize("block 1", 1, 10);
                context->wait();
                ASSERT_TRUE(f.isCompleted());
                EXPECT_TRUE(f.getValue().getValue().isSuccess());
            }

            TEST_F(BlockSyncTest, TwoBlocks) {
                SetUp(1, 1);
                setupFakeKeychains();
                std::vector<BL> bch =
                {
                    BL{ 1, "block 1",
                    {
                        TR{ { "X" },{ { "0", 10000 } } },
                    } },
                    BL{ 2, "block 2",
                    {
                        TR{ { "X" },{ { "0", 10000 } } },
                    } },
                };
                setBlockchain(bch);
                {
                    ::testing::Sequence s;
                    EXPECT_CALL(*blocksDBMock, addBlock(Truly(Same(bch[0]))));
                    EXPECT_CALL(*blocksDBMock, addBlock(Truly(Same(bch[1]))));
                }
                auto f = synchronizer->synchronize("block 1", 1, 10);
                context->wait();
                ASSERT_TRUE(f.isCompleted());
                EXPECT_TRUE(f.getValue().getValue().isSuccess());
            }

            TEST_F(BlockSyncTest, OthersTransactionsIgnored) {
                SetUp(1, 1);
                setupFakeKeychains();
                std::vector<BL> bch =
                {
                    BL{ 1, "block 1",
                    {
                        TR{ { "X" },{ { "0", 10000 } } },
                    } },
                    BL{ 2, "block 2",
                    {
                        TR{ { "X" },{ { "Y", 10000 } } },
                    } },
                };
                setBlockchain(bch);
                EXPECT_CALL(*blocksDBMock, addBlock(Truly(Same(bch[0])))).Times(1);
                EXPECT_CALL(*blocksDBMock, addBlock(Truly(Same(bch[1])))).Times(0);
                
                auto f = synchronizer->synchronize("block 1", 1, 10);
                context->wait();
                ASSERT_TRUE(f.isCompleted());
                EXPECT_TRUE(f.getValue().getValue().isSuccess());
            }

            TEST_F(BlockSyncTest, IgnoreTooNewBlocks) {
                SetUp(1, 1);
                setupFakeKeychains();
                std::vector<BL> bch =
                {
                    BL{ 1, "block 1",
                    {
                        TR{ { "X" },{ { "0", 10000 } } },
                    } },
                    BL{ 2, "block 2",
                    {
                        TR{ { "X" },{ { "0", 10000 } } },
                    } },
                };
                setBlockchain(bch);
                EXPECT_CALL(*blocksDBMock, addBlock(Truly(Same(bch[0])))).Times(1);
                EXPECT_CALL(*blocksDBMock, addBlock(Truly(Same(bch[1])))).Times(0);
                
                auto f = synchronizer->synchronize("block 1", 1, 1); // limiting the max block heigh
                context->wait();
                ASSERT_TRUE(f.isCompleted());
                EXPECT_TRUE(f.getValue().getValue().isSuccess());
            }

            TEST_F(BlockSyncTest, OneBlockDiscovery) {
                SetUp(1, 1);
                setupFakeKeychains();
                std::vector<BL> bch =
                {
                    BL{ 1, "block 1",
                    {
                        TR{ { "X" },{ { "0", 10000 } } },
                        TR{ { "X" },{ { "1", 10000 } } },
                        TR{ { "X" },{ { "2", 10000 } } },
                        TR{ { "X" },{ { "3", 10000 } } },
                        TR{ { "X" },{ { "4", 10000 } } },
                    } },
                };
                setBlockchain(bch);
                EXPECT_CALL(*blocksDBMock, addBlock(Truly(Same(bch[0]))));
                EXPECT_CALL(*keychainReceiveMock, markAsUsed(Eq("0")));
                EXPECT_CALL(*keychainReceiveMock, markAsUsed(Eq("1")));
                EXPECT_CALL(*keychainReceiveMock, markAsUsed(Eq("2")));
                EXPECT_CALL(*keychainReceiveMock, markAsUsed(Eq("3")));
                EXPECT_CALL(*keychainReceiveMock, markAsUsed(Eq("4")));
                EXPECT_CALL(*keychainReceiveMock, markAsUsed(Eq("5"))).Times(0); // not discovered
                auto f = synchronizer->synchronize("block 1", 1, 10);
                context->wait();
                ASSERT_TRUE(f.isCompleted());
                EXPECT_TRUE(f.getValue().getValue().isSuccess());
            }

            TEST_F(BlockSyncTest, SeveralBlocksDicovery) {
                SetUp(1, 1);
                setupFakeKeychains();
                std::vector<BL> bch =
                {
                    BL{ 1, "block 1",
                    {
                        TR{ { "X" },{ { "0", 10000 } } }
                    }},
                    BL{ 2, "block 2",
                    {
                        TR{ { "X" },{ { "1", 10000 } } }
                    } },
                    BL{ 3, "block 3",
                    {
                        TR{ { "X" },{ { "2", 10000 } } }
                    } },
                    BL{ 4, "block 4",
                    {
                        TR{ { "X" },{ { "3", 10000 } } }
                    } }
                };
                setBlockchain(bch);
                // Write blocks to the disk as soon as possible to not lose it in case of error
                {
                    ::testing::Sequence s;
                    for (int i = 0; i < 4; ++i)
                        EXPECT_CALL(*blocksDBMock, addBlock(Truly(Same(bch[i]))));
                }
                EXPECT_CALL(*keychainReceiveMock, markAsUsed(Eq("0")));
                EXPECT_CALL(*keychainReceiveMock, markAsUsed(Eq("1")));
                EXPECT_CALL(*keychainReceiveMock, markAsUsed(Eq("2")));
                EXPECT_CALL(*keychainReceiveMock, markAsUsed(Eq("3")));
                EXPECT_CALL(*keychainReceiveMock, markAsUsed(Eq("4"))).Times(0); // not discovered
                auto f = synchronizer->synchronize("block 1", 1, 10);
                context->wait();
                ASSERT_TRUE(f.isCompleted());
                EXPECT_TRUE(f.getValue().getValue().isSuccess());
            }

            TEST_F(BlockSyncTest, CheckTruncated) {
                SetUp(1, 1);
                const uint32_t LAST_BLOCK = 300;
                setupFakeKeychains();
                std::vector<BL> bch;
                std::vector<BL> res1;
                std::vector<BL> res2;
                {
                    ::testing::Sequence s;
                    for (uint32_t i = 1; i <= 300; ++i) {
                        auto b = BL{ i, "block " + boost::lexical_cast<std::string>(i),{ TR{ { "X" },{ { "0", 10000 } } } } };
                        bch.push_back(b);
                        EXPECT_CALL(*blocksDBMock, addBlock(Truly(Same(b))));
                    }
                }
                setBlockchain(bch);
                
                EXPECT_CALL(*keychainReceiveMock, markAsUsed(Eq("0"))).Times(AtLeast(1));
                EXPECT_CALL(*keychainReceiveMock, markAsUsed(Eq("1"))).Times(0); // not discovered
                auto f = synchronizer->synchronize("block 1", 1, LAST_BLOCK);
                context->wait();
                ASSERT_TRUE(f.isCompleted());
                EXPECT_TRUE(f.getValue().getValue().isSuccess());
            }

            TEST_F(BlockSyncTest, ExplorerError) {
                // Simulate a situation when we have more then TRUNCATION_LEVEL operations on one block
                // and this block is not the last one thet contain transactions.
                // This case is not implemented yet, it would require to make additional request to Explorer
                // to get next block hash.
                SetUp(1, 1);
                const uint32_t LAST_BLOCK = 300;
                const uint32_t TRUNCATION_LEVEL = 200;
                fakeExplorer.setTruncationLevel(TRUNCATION_LEVEL);
                setupFakeKeychains();
                std::vector<BL> bch;
                for (uint32_t i = 1; i <= 200; ++i) {
                    auto b = BL{ 1, "block 1",{ TR{ { "X" + boost::lexical_cast<std::string>(i) },{ { "0", 10000 } } } } };
                    bch.push_back(b);
                }
                bch.push_back(BL{ 2, "block 2",{ TR{ { "X" },{ { "0", 10000 } } } } });
                setBlockchain(bch);

                EXPECT_CALL(*blocksDBMock, addBlocks(_)).Times(0);
                EXPECT_CALL(*keychainReceiveMock, markAsUsed(_)).Times(0); // not discovered
                auto f = synchronizer->synchronize("block 1", 1, LAST_BLOCK);
                context->wait();
                ASSERT_TRUE(f.isCompleted());
                EXPECT_TRUE(f.getValue().getValue().isFailure());
                EXPECT_EQ(f.getValue().getValue().getFailure().getErrorCode(), api::ErrorCode::IMPLEMENTATION_IS_MISSING);
            }

            TEST_F(BlockSyncTest, LongBlockChain) {
                SetUp(20, 20);
                const uint32_t LAST_BLOCK = 300000;
                setupFakeKeychains();
                std::vector<BL> bch;
                {
                    ::testing::Sequence s;
                    for (uint32_t i = 1; i <= LAST_BLOCK; i += 500) {
                        auto b = BL{ i, "block " + boost::lexical_cast<std::string>(i),{ TR{ { "X" },{ { boost::lexical_cast<std::string>((i / 500) % 500), 10000 } } } } };
                        bch.push_back(b);
                        EXPECT_CALL(*blocksDBMock, addBlock(Truly(Same(b)))).Times(1);
                    }
                }
                setBlockchain(bch);
                EXPECT_CALL(*keychainReceiveMock, markAsUsed(_)).Times(AtLeast(1));
                EXPECT_CALL(*keychainReceiveMock, markAsUsed(Eq("499"))).Times(AtLeast(1));
                EXPECT_CALL(*keychainReceiveMock, markAsUsed(Eq("500"))).Times(0); // not discovered
                auto f = synchronizer->synchronize(bch[0].hash, bch[0].height, LAST_BLOCK);
                context->wait();
                ASSERT_TRUE(f.isCompleted());
                EXPECT_TRUE(f.getValue().getValue().isSuccess());
            }
        }
    }
}