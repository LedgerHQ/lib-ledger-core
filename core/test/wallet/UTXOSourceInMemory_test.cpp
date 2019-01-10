#include <CommonFixtureFunctions.hpp>
#include <Mocks.hpp>
#include <Helpers.hpp>
#include <async/Future.hpp>
#include <gtest/gtest.h>
#include <memory>
#include <wallet/common/InMemoryBlockchainDatabase.hpp>
#include <wallet/bitcoin/UTXOSourceInMemory.hpp>

using namespace ledger::core;
using namespace ledger::core::common;
using namespace ledger::core::bitcoin;
using namespace ledger::core::tests;
using namespace ::testing;

struct UTXOSourceInMemoryFixture: virtual Test, CommonFixtureFunctions {
protected:
    std::shared_ptr<SimpleExecutionContext> _ctx;
    std::shared_ptr<NiceMock<BlocksDBMock>> _blocksDB;
    std::shared_ptr<InMemoryBlockchainDatabase<BitcoinLikeNetwork::FilledBlock>> _fakeDB;
    std::shared_ptr<KeychainRegistryMock> _keychainRegistry;
    std::shared_ptr<UTXOSourceInMemory> _source;

public:
    UTXOSourceInMemoryFixture()
        : _ctx(std::make_shared<SimpleExecutionContext>()),
          _blocksDB(std::make_shared<NiceMock<BlocksDBMock>>()),
          _fakeDB(std::make_shared<InMemoryBlockchainDatabase<BitcoinLikeNetwork::FilledBlock>>(_ctx)),
          _keychainRegistry(std::make_shared<KeychainRegistryMock>()),
          _source(std::make_shared<UTXOSourceInMemory>(
              _blocksDB,
              _keychainRegistry
          )) {
        linkMockDbToFake(_blocksDB, *_fakeDB);
    }
};

TEST_F(UTXOSourceInMemoryFixture, NoInitialUTXO) {
    auto future = _source->getUTXOs(_ctx);
    _ctx->wait();
    auto sourceList = getFutureResult(future);

    EXPECT_TRUE(sourceList.available.empty());
    EXPECT_TRUE(sourceList.spent.empty());
}

// Test that the algorithm that prunes UTXO is working properly.
TEST_F(UTXOSourceInMemoryFixture, PruneUsedUTXO) {
    // create a block that contains a single transaction
    EXPECT_CALL(*_keychainRegistry, containsAddress(_)).WillRepeatedly(Return(true));

    _fakeDB->addBlock(17, toFilledBlock(
        BL{ 17, "block 17",
            {
                TR{ { { "X", 0 } }, { { "0", 10000 } } }
            }
        }
    ));

    auto future = _source->getUTXOs(_ctx);
    _ctx->wait();

    auto sourceList = getFutureResult(future);
    std::map<UTXOKey, UTXOValue> available = { { std::make_pair("block 17TR{X,}->{0,}", 0), UTXOValue(BigInt(10000), "0") } };
    std::set<UTXOKey> spent = { { "X", 0 } };
    EXPECT_EQ(sourceList.available, available);
    EXPECT_EQ(sourceList.spent, spent);

    // create a transaction that consumes the previous output
    _blocksDB->addBlock(18, toFilledBlock(
        BL{ 18, "block 2",
            {
                TR{ { { "block 17TR{X,}->{0,}", 0 } }, { { "1", 3141592 } } }
            }
        }
    ));

    auto future2 = _source->getUTXOs(_ctx);
    _ctx->wait();

    auto sourceList2 = getFutureResult(future2);
    std::map<UTXOKey, UTXOValue> available2 = { { std::make_pair("block 2TR{block 17TR{X,}->{0,},}->{1,}", 0), UTXOValue(BigInt(3141592), "1") } };
    std::set<UTXOKey> spent2 = { { "X", 0 } };

    EXPECT_EQ(sourceList2.available, available2); // UTXO consumed
    EXPECT_EQ(sourceList2.spent, spent2);
}
