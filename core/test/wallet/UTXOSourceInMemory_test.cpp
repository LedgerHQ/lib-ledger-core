#include <Mocks.hpp>
#include <Helpers.hpp>
#include <gtest/gtest.h>
#include <memory>
#include <wallet/bitcoin/UTXOSourceInMemory.hpp>

using namespace ledger::core;
using namespace ledger::core::bitcoin;
using namespace ledger::core::tests;

struct UTXOSourceInMemoryTest: ::testing::Test {
protected:
    std::shared_ptr<SimpleExecutionContext> _ctx;
    UTXOSourceInMemory _source;

public:
    UTXOSourceInMemoryTest()
        : _ctx(new SimpleExecutionContext()),
          _source(std::make_shared(BlocksDBMock),
                  std::make_shared(KeychainRegistryMock)) {
    }
}

struct UTXOSourceInMemorySimpleTest: UTXOSourceInMemoryTest;

TEST_F(UTXOSourceInMemorySimpleTest, NoInitialUTXO) {
    EXPECT_TRUE(false);
}
