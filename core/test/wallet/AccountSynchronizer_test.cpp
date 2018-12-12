#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <database/BlockchainDBInMemory.hpp>
#include <memory>
#include <wallet/NetworkTypes.hpp>
#include <wallet/common/AccountSynchronizer.hpp>
#include <events/ProgressNotifier.h>
#include <wallet/common/InMemoryPartialBlocksDB.hpp>
#include <wallet/common/PersistentBlockchainDatabase.hpp>
#include <vector>
#include <spdlog/spdlog.h>

#include "Helpers.hpp"
#include "Mocks.hpp"

using namespace testing;

namespace ledger {
    namespace core {
        namespace tests {
            class AccountSyncTest : public ::testing::Test {
            public:
                AccountSyncTest()
                    : receivedFake(0, 0) // receive addresses are 0,1,2... change are 1000, 1001...
                    , changeFake(0, 1000) {
                    explorerMock = std::make_shared<NiceMock<ExplorerMock>>();
                    keychainReceiveMock = std::make_shared<NiceMock<KeychainMock>>();
                    keychainChangeMock = std::make_shared<NiceMock<KeychainMock>>();
                    stableBlocksDBMock = std::make_shared<NiceMock<BlocksDBMock>>();
                    unstableBlocksDBMock = std::make_shared<NiceMock<BlocksDBMock>>();
                    context = std::make_shared<SimpleExecutionContext>();
                    std::shared_ptr<api::ExecutionContext> xx = context;
                    loggerSinkMock = std::make_shared<NiceMock<LoggerSinkMock>>();
                    logger = std::make_shared<spdlog::logger>("unittestlogger", loggerSinkMock);
                    fakeStableDBBackend = std::make_shared<db::BlockchainDBInMemory>();
                    fakeUnstableDBBackend = std::make_shared<db::BlockchainDBInMemory>();
                    fakeStableDB = std::make_shared<common::PersistentBlockchainDatabase<BitcoinLikeNetwork>>(context, fakeStableDBBackend);
                    fakeUnstableDB = std::make_shared<common::PersistentBlockchainDatabase<BitcoinLikeNetwork>>(context, fakeUnstableDBBackend);
                }

                void SetUp(uint32_t maxPossibleUnstableBlocks, const std::string& genesisBlockHash ) {
                    common::SynchronizerConfiguration config(
                        maxPossibleUnstableBlocks,
                        1,
                        1,
                        200,
                        genesisBlockHash);
                   
                    synchronizer = std::make_shared<common::AccountSynchronizer<BitcoinLikeNetwork>>(
                        context,
                        explorerMock,
                        stableBlocksDBMock,
                        unstableBlocksDBMock,
                        keychainReceiveMock,
                        keychainChangeMock,
                        logger,
                        config);
                }

                void setupFakeKeychains() {
                    ON_CALL(*keychainReceiveMock, getNumberOfUsedAddresses()).WillByDefault(Invoke(&receivedFake, &FakeKeyChain::getNumberOfUsedAddresses));
                    ON_CALL(*keychainReceiveMock, getAddresses(_, _)).WillByDefault(Invoke(&receivedFake, &FakeKeyChain::getAddresses));
                    ON_CALL(*keychainReceiveMock, markAsUsed(_)).WillByDefault(Invoke(&receivedFake, &FakeKeyChain::markAsUsed));
                    ON_CALL(*keychainChangeMock, getNumberOfUsedAddresses()).WillByDefault(Invoke(&changeFake, &FakeKeyChain::getNumberOfUsedAddresses));
                    ON_CALL(*keychainChangeMock, getAddresses(_, _)).WillByDefault(Invoke(&changeFake, &FakeKeyChain::getAddresses));
                    ON_CALL(*keychainChangeMock, markAsUsed(_)).WillByDefault(Invoke(&changeFake, &FakeKeyChain::markAsUsed));
                };

                void setupFakeDatabases() {
                    stableBlocksDBMock->linkWithFake(fakeStableDB);
                }

                void setBlockchain(const std::vector<BL>& blockChain) {
                    fakeExplorer.setBlockchain(toFilledBlocks(blockChain));
                    ON_CALL(*explorerMock, getTransactions(_, _, _)).WillByDefault(Invoke(&fakeExplorer, &FakeExplorer::getTransactions));
                }

            public:
                std::shared_ptr<core::AccountSynchronizer<BitcoinLikeNetwork>> synchronizer;
                std::shared_ptr<NiceMock<ExplorerMock>> explorerMock;
                std::shared_ptr<NiceMock<KeychainMock>> keychainReceiveMock;
                std::shared_ptr<NiceMock<KeychainMock>> keychainChangeMock;
                std::shared_ptr<NiceMock<BlocksDBMock>> stableBlocksDBMock;
                std::shared_ptr<NiceMock<BlocksDBMock>> unstableBlocksDBMock;
                std::shared_ptr<SimpleExecutionContext> context;
                std::shared_ptr<spdlog::logger> logger;
                std::shared_ptr<NiceMock<LoggerSinkMock>> loggerSinkMock;
                FakeKeyChain receivedFake;
                FakeKeyChain changeFake;
                FakeExplorer fakeExplorer;
                std::shared_ptr<common::PersistentBlockchainDatabase<BitcoinLikeNetwork>> fakeStableDB;
                std::shared_ptr<common::PersistentBlockchainDatabase<BitcoinLikeNetwork>> fakeUnstableDB;
            private:
                std::shared_ptr<db::BlockchainDBInMemory> fakeStableDBBackend;
                std::shared_ptr<db::BlockchainDBInMemory> fakeUnstableDBBackend;
            };

            TEST_F(AccountSyncTest, OneTransaction) {
                SetUp(1, "block 1");
                setupFakeKeychains();
                std::vector<BL> bch =
                {
                    BL{ 1, "block 1",
                    {
                        TR{ { "X" },{ { "0", 10000 } } }
                    } }
                };
                setBlockchain(bch);
                auto f = synchronizer->synchronize()->getFuture();
                context->wait();
                ASSERT_TRUE(f.isCompleted());
                EXPECT_TRUE(f.getValue().getValue().isSuccess());
            }
        }
    }
}