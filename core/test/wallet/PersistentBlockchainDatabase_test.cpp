#include <api/ExecutionContext.hpp>
#include <gtest/gtest.h>
#include <Helpers.hpp>
#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include <Mocks.hpp>
#include <wallet/common/PersistentBlockchainDatabase.hpp>
#include <wallet/NetworkTypes.hpp>

using namespace ledger::core;
using namespace ledger::core::tests;

class PersistentBlockchainDatabaseTest : public ::testing::Test {
public:
    PersistentBlockchainDatabaseTest()
    : backendMock(new BlockchainDBMock())
    , context(new SimpleExecutionContext())
    , db(context, backendMock) {
    }
    
public:
    std::shared_ptr<db::BlockchainDB> backendMock;
    std::shared_ptr<api::ExecutionContext> context;

    common::PersistentBlockchainDatabase<BitcoinLikeNetwork> db;
};

TEST_F(PersistentBlockchainDatabaseTest, SimpleGet) {

}