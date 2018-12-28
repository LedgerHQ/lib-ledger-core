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
    std::shared_ptr<UTXOSourceInMemory> _source;

public:
    UTXOSourceInMemoryFixture()
        : _ctx(std::make_shared<SimpleExecutionContext>()),
          _blocksDB(std::make_shared<NiceMock<BlocksDBMock>>()),
          _fakeDB(std::make_shared<InMemoryBlockchainDatabase<BitcoinLikeNetwork::FilledBlock>>(_ctx)),
          _source(std::make_shared<UTXOSourceInMemory>(
              _blocksDB,
              std::make_shared<KeychainRegistryMock>()
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
