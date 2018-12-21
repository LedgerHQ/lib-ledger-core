#include <api/ExecutionContext.hpp>
#include <gtest/gtest.h>
#include <Helpers.hpp>
#include <Mocks.hpp>
#include <map>
#include <wallet/bitcoin/BalanceServiceUTXOBased.hpp>

using namespace ledger::core;
using namespace ledger::core::tests;
using namespace testing;

class BalanceServiceUTXOBasedTest : public Test {
public:
    typedef std::map<bitcoin::UTXOKey, bitcoin::UTXOValue> KV;
public:
    BalanceServiceUTXOBasedTest() {
        context = std::make_shared<SimpleExecutionContext>();
        utxoServiceMock = std::make_shared<UTXOServiceMock>();
        balanceService = std::make_shared<bitcoin::BalanceServiceUTXOBased>(context, utxoServiceMock);
    };

    void setupUTXOService(const KV& utxos) {
        EXPECT_CALL(*utxoServiceMock, getUTXOs()).WillOnce(Return(Future<KV>::successful(utxos)));
    }
    
public:
    std::shared_ptr<SimpleExecutionContext> context;
    std::shared_ptr<UTXOServiceMock> utxoServiceMock;
    std::shared_ptr<bitcoin::BalanceServiceUTXOBased> balanceService;
};

TEST_F(BalanceServiceUTXOBasedTest, SimpleTest) {
    setupUTXOService(
    {
        {{"TX1", 0}, {BigInt(10), "address1"}}
    });
    auto f = balanceService->getBalance();
    context->wait();
    ASSERT_EQ(BigInt(10), getFutureResult(f));
}
