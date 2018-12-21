#include <api/ExecutionContext.hpp>
#include <gtest/gtest.h>
#include <Helpers.hpp>
#include <Mocks.hpp>
#include <map>
#include <wallet/bitcoin/UTXOServiceSourceBased.hpp>

using namespace ledger::core;
using namespace ledger::core::tests;
using namespace testing;

class UTXOServiceSourceBasedTest : public Test {
public:
    typedef std::map<bitcoin::UTXOKey, bitcoin::UTXOValue> KV;
public:
    UTXOServiceSourceBasedTest()
        : dummy{ BigInt(10), "address1" } {
        context = std::make_shared<SimpleExecutionContext>();
        stable = std::make_shared<UTXOSourceMock>();
        unstable = std::make_shared<UTXOSourceMock>();
        pending = std::make_shared<UTXOSourceMock>();
        utxoService = std::make_shared<bitcoin::UTXOServiceSourceBased>(context, stable, unstable, pending);
    };

    static void setupSource(std::shared_ptr<UTXOSourceMock>& mock, const bitcoin::UTXOSourceList& list) {
        EXPECT_CALL(*mock, getUTXOs(_)).WillOnce(Return(Future<bitcoin::UTXOSourceList>::successful(list)));
    }
    
public:
    std::shared_ptr<SimpleExecutionContext> context;
    std::shared_ptr<UTXOSourceMock> stable;
    std::shared_ptr<UTXOSourceMock> unstable;
    std::shared_ptr<UTXOSourceMock> pending;
    bitcoin::UTXOValue dummy;
    std::shared_ptr<bitcoin::UTXOServiceSourceBased> utxoService;
};

TEST_F(UTXOServiceSourceBasedTest, EmptySources) {
    setupSource(stable, { {}, {} });
    setupSource(unstable, { {}, {} });
    setupSource(pending, { {}, {} });
    auto f = utxoService->getUTXOs();
    context->wait();
    std::map<bitcoin::UTXOKey, bitcoin::UTXOValue> empty;
    ASSERT_THAT(empty, ContainerEq(getFutureResult(f)));
}

TEST_F(UTXOServiceSourceBasedTest, OnlyStable) {
    setupSource(stable,
    { 
        {
            { {"TX1", 0}, dummy},
            { {"TX1", 1}, dummy},
            { {"TX2", 0}, dummy}
        },
    {} });
    setupSource(unstable, { {}, {} });
    setupSource(pending, { {}, {} });
    auto f = utxoService->getUTXOs();
    context->wait();
    KV expected = 
    {
        { { "TX1", 0 }, dummy },
        { { "TX1", 1 }, dummy },
        { { "TX2", 0 }, dummy }
    };
    ASSERT_THAT(expected, ContainerEq(getFutureResult(f)));
}

TEST_F(UTXOServiceSourceBasedTest, OnlyUnStable) {
    setupSource(stable, { {},{} });
    setupSource(unstable,
    {
        {
            { { "TX1", 0 }, dummy },
            { { "TX1", 1 }, dummy },
            { { "TX2", 0 }, dummy }
        },
        {} });
    setupSource(pending, { {}, {} });
    auto f = utxoService->getUTXOs();
    context->wait();
    KV expected =
    {
        { { "TX1", 0 }, dummy },
        { { "TX1", 1 }, dummy },
        { { "TX2", 0 }, dummy }
    };
    ASSERT_THAT(expected, ContainerEq(getFutureResult(f)));
}

TEST_F(UTXOServiceSourceBasedTest, OnlyPending) {
    setupSource(stable, { {}, {} });
    setupSource(unstable, { {}, {} });
    setupSource(pending,
    {
        {
            { { "TX1", 0 }, dummy },
            { { "TX1", 1 }, dummy },
            { { "TX2", 0 }, dummy }
        },
        {} });
    auto f = utxoService->getUTXOs();
    context->wait();
    KV expected =
    {
        { { "TX1", 0 }, dummy },
        { { "TX1", 1 }, dummy },
        { { "TX2", 0 }, dummy }
    };
    ASSERT_THAT(expected, ContainerEq(getFutureResult(f)));
}

TEST_F(UTXOServiceSourceBasedTest, NoIntersection) {
    setupSource(stable, { { { { "TX1", 1 }, dummy } }, {} });
    setupSource(unstable, { { { { "TX1", 0 }, dummy } }, {} });
    setupSource(pending, { { { { "TX2", 0 }, dummy } }, {} });
    auto f = utxoService->getUTXOs();
    context->wait();
    KV expected =
    {
        { { "TX1", 0 }, dummy },
        { { "TX1", 1 }, dummy },
        { { "TX2", 0 }, dummy }
    };
    ASSERT_THAT(expected, ContainerEq(getFutureResult(f)));
}

TEST_F(UTXOServiceSourceBasedTest, PendingSpendFromStable) {
    setupSource(stable, { 
        { 
            { { "TX1", 1 }, dummy },
            { { "TX2", 0 }, dummy },
            { { "TX2", 1 }, dummy },
            { { "TX2", 2 }, dummy }
        }, {} });
    setupSource(unstable, { {}, {} });
    setupSource(pending, { {}, 
    {
        { "TX1", 1 },
        { "TX2", 1 },
    } 
    });
    auto f = utxoService->getUTXOs();
    context->wait();
    KV expected =
    {
        { { "TX2", 0 }, dummy },
        { { "TX2", 2 }, dummy },
    };
    ASSERT_THAT(expected, ContainerEq(getFutureResult(f)));
}

TEST_F(UTXOServiceSourceBasedTest, PendingSpendFromUnStable) {
    setupSource(stable, { {},{} });
    setupSource(unstable, {
        {
            { { "TX1", 1 }, dummy },
            { { "TX2", 0 }, dummy },
            { { "TX2", 1 }, dummy },
            { { "TX2", 2 }, dummy }
        },{} });
    setupSource(pending, { {},
    {
        { "TX1", 1 },
        { "TX2", 1 },
    }
    });
    auto f = utxoService->getUTXOs();
    context->wait();
    KV expected =
    {
        { { "TX2", 0 }, dummy },
        { { "TX2", 2 }, dummy },
    };
    ASSERT_THAT(expected, ContainerEq(getFutureResult(f)));
}

TEST_F(UTXOServiceSourceBasedTest, UnStableSpendFromStable) {
    setupSource(stable, {
        {
            { { "TX1", 1 }, dummy },
            { { "TX2", 0 }, dummy },
            { { "TX2", 1 }, dummy },
            { { "TX2", 2 }, dummy }
        },{} });
    setupSource(unstable, { {},
    {
        { "TX1", 1 },
        { "TX2", 1 },
    }
    });
    setupSource(pending, { {},{} });
    auto f = utxoService->getUTXOs();
    context->wait();
    KV expected =
    {
        { { "TX2", 0 }, dummy },
        { { "TX2", 2 }, dummy },
    };
    ASSERT_THAT(expected, ContainerEq(getFutureResult(f)));
}

TEST_F(UTXOServiceSourceBasedTest, SameUTXOs) {
    setupSource(stable, {
        {
            { { "TX1", 1 }, dummy },
            { { "TX2", 0 }, dummy },
            { { "TX2", 1 }, dummy },
            { { "TX2", 2 }, dummy }
        },{} });
    setupSource(unstable, {
        {
            { { "TX1", 1 }, dummy },
            { { "TX2", 0 }, dummy },
            { { "TX2", 1 }, dummy },
            { { "TX2", 2 }, dummy }
        },{} });
    setupSource(pending, {
        {
            { { "TX1", 1 }, dummy },
            { { "TX2", 0 }, dummy },
            { { "TX2", 1 }, dummy },
            { { "TX2", 2 }, dummy }
        },{} });
    auto f = utxoService->getUTXOs();
    context->wait();
    KV expected =
    {
        { { "TX1", 1 }, dummy },
        { { "TX2", 0 }, dummy },
        { { "TX2", 1 }, dummy },
        { { "TX2", 2 }, dummy }
    };
    ASSERT_THAT(expected, ContainerEq(getFutureResult(f)));
}
