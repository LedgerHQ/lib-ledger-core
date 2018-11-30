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
                    firstBlock = std::make_shared<BitcoinLikeNetwork::Block>();
                    lastBlock = std::make_shared<BitcoinLikeNetwork::Block>();
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

                void setupDBThatIsAllwaysHappy() {
                    ON_CALL(*blocksDBMock, addBlocks(_)).WillByDefault(Invoke([](const std::vector<BitcoinLikeNetwork::FilledBlock>& x) {return Future<Unit>::successful(unit); }));
                }

                std::function<bool(const std::vector<BitcoinLikeNetwork::FilledBlock>& x)> SameBlocks(const std::vector<BL>& bch) {
                     return [bch](const std::vector<BitcoinLikeNetwork::FilledBlock>& x) { return blocksSame(x, bch); };
                };

            public:
                std::shared_ptr<common::BlocksSynchronizer<BitcoinLikeNetwork>> synchronizer;
                std::shared_ptr<BitcoinLikeNetwork::Block> firstBlock;
                std::shared_ptr<BitcoinLikeNetwork::Block> lastBlock;
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
                setupDBThatIsAllwaysHappy();
                setupFakeKeychains();
                std::vector<BL> bch =
                {
                    BL{ 1, "block 1",
                    {
                        TR{ { "X" },{ { "0", 10000 } } }
                    }}
                };
                setBlockchain(bch);
                EXPECT_CALL(*blocksDBMock, addBlocks(Truly(SameBlocks(bch))));
                firstBlock->hash = "block 1";
                firstBlock->height = 1;
                lastBlock->height = 10;
                auto f = synchronizer->synchronize(firstBlock, lastBlock);
                context->wait();
                ASSERT_TRUE(f.isCompleted());
                EXPECT_TRUE(f.getValue().getValue().isSuccess());
            }

            TEST_F(BlockSyncTest, TwoTransOneBlock) {
                SetUp(1, 1);
                setupDBThatIsAllwaysHappy();
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
                EXPECT_CALL(*blocksDBMock, addBlocks(Truly(SameBlocks(bch))));
                firstBlock->hash = "block 1";
                firstBlock->height = 1;
                lastBlock->height = 10;
                auto f = synchronizer->synchronize(firstBlock, lastBlock);
                context->wait();
                ASSERT_TRUE(f.isCompleted());
                EXPECT_TRUE(f.getValue().getValue().isSuccess());
            }

            TEST_F(BlockSyncTest, TwoBlocks) {
                SetUp(1, 1);
                setupDBThatIsAllwaysHappy();
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
                EXPECT_CALL(*blocksDBMock, addBlocks(Truly(SameBlocks(bch))));
                firstBlock->hash = "block 1";
                firstBlock->height = 1;
                lastBlock->height = 10;
                auto f = synchronizer->synchronize(firstBlock, lastBlock);
                context->wait();
                ASSERT_TRUE(f.isCompleted());
                EXPECT_TRUE(f.getValue().getValue().isSuccess());
            }

            TEST_F(BlockSyncTest, OthersTransactionsIgnored) {
                SetUp(1, 1);
                setupDBThatIsAllwaysHappy();
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
                EXPECT_CALL(*blocksDBMock, addBlocks(Truly(SameBlocks(
                {
                    BL{ 1, "block 1",
                    {
                        TR{ { "X" },{ { "0", 10000 } } },
                    } }
                }
                ))));
                firstBlock->hash = "block 1";
                firstBlock->height = 1;
                lastBlock->height = 10;
                auto f = synchronizer->synchronize(firstBlock, lastBlock);
                context->wait();
                ASSERT_TRUE(f.isCompleted());
                EXPECT_TRUE(f.getValue().getValue().isSuccess());
            }

            TEST_F(BlockSyncTest, IgnoreTooNewBlocks) {
                SetUp(1, 1);
                setupDBThatIsAllwaysHappy();
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
                EXPECT_CALL(*blocksDBMock, addBlocks(Truly(SameBlocks(
                {
                    BL{ 1, "block 1",
                    {
                        TR{ { "X" },{ { "0", 10000 } } },
                    } }
                }
                ))));
                firstBlock->hash = "block 1";
                firstBlock->height = 1;
                lastBlock->height = 1; // limiting the max block heigh
                auto f = synchronizer->synchronize(firstBlock, lastBlock);
                context->wait();
                ASSERT_TRUE(f.isCompleted());
                EXPECT_TRUE(f.getValue().getValue().isSuccess());
            }

            TEST_F(BlockSyncTest, OneBlockDiscovery) {
                SetUp(1, 1);
                setupDBThatIsAllwaysHappy();
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
                EXPECT_CALL(*blocksDBMock, addBlocks(Truly(SameBlocks(bch))));
                EXPECT_CALL(*keychainReceiveMock, markAsUsed(Eq("0")));
                EXPECT_CALL(*keychainReceiveMock, markAsUsed(Eq("1")));
                EXPECT_CALL(*keychainReceiveMock, markAsUsed(Eq("2")));
                EXPECT_CALL(*keychainReceiveMock, markAsUsed(Eq("3")));
                EXPECT_CALL(*keychainReceiveMock, markAsUsed(Eq("4")));
                EXPECT_CALL(*keychainReceiveMock, markAsUsed(Eq("5"))).Times(0); // not discovered
                firstBlock->hash = "block 1";
                firstBlock->height = 1;
                lastBlock->height = 10;
                auto f = synchronizer->synchronize(firstBlock, lastBlock);
                context->wait();
                ASSERT_TRUE(f.isCompleted());
                EXPECT_TRUE(f.getValue().getValue().isSuccess());
            }

            TEST_F(BlockSyncTest, SeveralBlocksDicovery) {
                SetUp(1, 1);
                setupDBThatIsAllwaysHappy();
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
                EXPECT_CALL(*blocksDBMock, addBlocks(Truly(SameBlocks({ BL{ 1, "block 1",{ TR{ { "X" },{ { "0", 10000 } } } } } }))));
                EXPECT_CALL(*blocksDBMock, addBlocks(Truly(SameBlocks({ BL{ 2, "block 2",{ TR{ { "X" },{ { "1", 10000 } } } } } }))));
                EXPECT_CALL(*blocksDBMock, addBlocks(Truly(SameBlocks({ BL{ 3, "block 3",{ TR{ { "X" },{ { "2", 10000 } } } } } }))));
                EXPECT_CALL(*blocksDBMock, addBlocks(Truly(SameBlocks({ BL{ 4, "block 4",{ TR{ { "X" },{ { "3", 10000 } } } } } }))));

                EXPECT_CALL(*keychainReceiveMock, markAsUsed(Eq("0")));
                EXPECT_CALL(*keychainReceiveMock, markAsUsed(Eq("1")));
                EXPECT_CALL(*keychainReceiveMock, markAsUsed(Eq("2")));
                EXPECT_CALL(*keychainReceiveMock, markAsUsed(Eq("3")));
                EXPECT_CALL(*keychainReceiveMock, markAsUsed(Eq("4"))).Times(0); // not discovered
                firstBlock->hash = "block 1";
                firstBlock->height = 1;
                lastBlock->height = 10;
                auto f = synchronizer->synchronize(firstBlock, lastBlock);
                context->wait();
                ASSERT_TRUE(f.isCompleted());
                EXPECT_TRUE(f.getValue().getValue().isSuccess());
            }

            TEST_F(BlockSyncTest, CheckTruncated) {
                SetUp(1, 1);
                const uint32_t LAST_BLOCK = 300;
                setupDBThatIsAllwaysHappy();
                setupFakeKeychains();
                std::vector<BL> bch;
                std::vector<BL> res1;
                std::vector<BL> res2;
                for (uint32_t i = 1; i <= 300; ++i) {
                    auto b = BL{ i, "block " + boost::lexical_cast<std::string>(i),{ TR{ { "X" },{ { "0", 10000 } } } } };
                    bch.push_back(b);
                    if (i < 200)
                        res1.push_back(b);
                    else
                        res2.push_back(b);
                }
                setBlockchain(bch);
                
                firstBlock->hash = "block 1";
                firstBlock->height = 1;
                lastBlock->height = LAST_BLOCK;

                EXPECT_CALL(*blocksDBMock, addBlocks(Truly(SameBlocks(res1))));
                EXPECT_CALL(*blocksDBMock, addBlocks(Truly(SameBlocks(res2))));
                EXPECT_CALL(*keychainReceiveMock, markAsUsed(Eq("0"))).Times(AtLeast(1));
                EXPECT_CALL(*keychainReceiveMock, markAsUsed(Eq("1"))).Times(0); // not discovered
                auto f = synchronizer->synchronize(firstBlock, lastBlock);
                context->wait();
                ASSERT_TRUE(f.isCompleted());
                EXPECT_TRUE(f.getValue().getValue().isSuccess());
            }

            TEST_F(BlockSyncTest, ExplorerError) {
                SetUp(1, 1);
                const uint32_t LAST_BLOCK = 300;
                const uint32_t TRUNCATION_LEVEL = 100;
                fakeExplorer.setTruncationLevel(TRUNCATION_LEVEL);
                setupDBThatIsAllwaysHappy();
                setupFakeKeychains();
                std::vector<BL> bch;
                for (uint32_t i = 1; i <= 300; ++i) {
                    auto b = BL{ i, "block " + boost::lexical_cast<std::string>(i),{ TR{ { "X" },{ { "0", 10000 } } } } };
                    bch.push_back(b);
                }
                setBlockchain(bch);
                firstBlock->hash = "block 1";
                firstBlock->height = 1;
                lastBlock->height = LAST_BLOCK;

                EXPECT_CALL(*blocksDBMock, addBlocks(_)).Times(0);
                EXPECT_CALL(*keychainReceiveMock, markAsUsed(_)).Times(0); // not discovered
                auto f = synchronizer->synchronize(firstBlock, lastBlock);
                context->wait();
                ASSERT_TRUE(f.isCompleted());
                EXPECT_TRUE(f.getValue().getValue().isFailure());
                EXPECT_EQ(f.getValue().getValue().getFailure().getErrorCode(), api::ErrorCode::API_ERROR);
            }

            TEST_F(BlockSyncTest, LongBlockChain) {
                SetUp(20, 20);
                const uint32_t LAST_BLOCK = 3000000;
                setupDBThatIsAllwaysHappy();
                setupFakeKeychains();
                std::vector<BL> bch;
                for (uint32_t i = 1; i <= LAST_BLOCK; i += 500) {
                    auto b = BL{ i, "block " + boost::lexical_cast<std::string>(i),{ TR{ { "X" },{ { boost::lexical_cast<std::string>((i/500)% 500), 10000 } } } } };
                    bch.push_back(b);
                }
                setBlockchain(bch);
                firstBlock->hash = bch[0].hash;
                firstBlock->height = bch[0].height;
                lastBlock->height = LAST_BLOCK;
                EXPECT_CALL(*keychainReceiveMock, markAsUsed(_)).Times(AtLeast(1));
                EXPECT_CALL(*keychainReceiveMock, markAsUsed(Eq("499"))).Times(AtLeast(1));
                EXPECT_CALL(*keychainReceiveMock, markAsUsed(Eq("500"))).Times(0); // not discovered
                auto f = synchronizer->synchronize(firstBlock, lastBlock);
                context->wait();
                ASSERT_TRUE(f.isCompleted());
                EXPECT_TRUE(f.getValue().getValue().isSuccess());
            }
        }
    }
}