/*
 *
 * synchronization_btc_rbf_tests.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 13/06/2020.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Ledger
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <gtest/gtest.h>
#include "../BaseFixture.h"
#include <set>
#include <api/KeychainEngines.hpp>
#include <utils/DateUtils.hpp>
#include <wallet/bitcoin/database/BitcoinLikeAccountDatabaseHelper.h>
#include <wallet/bitcoin/api_impl/BitcoinLikeTransactionApi.h>
#include "ExplorerStorage.hpp"
#include "HttpClientOnFakeExplorer.hpp"
#include <UvThreadDispatcher.hpp>
#include <boost/algorithm/string/join.hpp>
#include <algorithm>

struct BitcoinLikeWalletBtcRbfSynchronization : public BaseFixture {

    void SetUp() override {
        BaseFixture::SetUp();
        explorer = std::make_shared<test::ExplorerStorage>();
        backend = std::static_pointer_cast<DatabaseBackend>(DatabaseBackend::getSqlite3Backend());
    }

    void TearDown() override {
        BaseFixture::TearDown();
        explorer = nullptr;
    }

    std::shared_ptr<WalletPool> newPool() {
        auto dispatcher = uv::createDispatcher();
        printer = std::make_shared<CoutLogPrinter>(dispatcher->getMainExecutionContext());
        return WalletPool::newInstance(
                "rbf_pool",
                "test",
                std::make_shared<test::HttpClientOnFakeExplorer>(explorer),
                nullptr,
                resolver,
                printer,
                dispatcher,
                rng,
                backend,
                api::DynamicObject::newInstance()
        );
    }

    void synchronizeAccount(const std::shared_ptr<BitcoinLikeAccount>& account) {
        auto bus = account->synchronize();
        Promise<Unit> promise;
        bus->subscribe(dispatcher->getSerialExecutionContext("worker"),
                       make_receiver([=](const std::shared_ptr<api::Event>& event) mutable {
                           if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                               return;
                           promise.success(unit);
                       }));
        uv::wait(promise.getFuture());
    }

    std::vector<std::shared_ptr<api::Operation>> getAllOperartions(const std::shared_ptr<BitcoinLikeAccount>& account) {
        return uv::wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())->execute());
    }

    std::shared_ptr<test::ExplorerStorage> explorer;
};

struct Output {
    uint64_t value;
    std::string address;

    Output(uint64_t val, const std::string& addr) : value(val), address(addr) {}

    std::string toJson(int index) const {
        return fmt::format(R"({{"output_index": {}, "address": "{}", "value": {}, "script_hex": "0000"}})", index, address, value);
    }
};

struct Input {
    std::string outputHash;
    int outputIndex;
    Output prevOutput;
    uint64_t sequence;

    Input(const std::string& hash, int index, const Output& output, uint64_t seq = 0xFFFFFFFF)
        : outputHash(hash), outputIndex(index), prevOutput(output), sequence(seq) {

    }

    std::string toJson(int index) const {
        return fmt::format(R"({{"input_index": {}, "output_hash": "{}", "output_index": {}, "value": {}, "address": "{}", "sequence": {}, "script_signature": "0000"}})",
            index, outputHash, outputIndex, prevOutput.value, prevOutput.address,
            sequence
        );
    }

};

struct Block {
    std::string hash;
    uint32_t height;

    Block(const std::string& hsh, uint32_t ht) : hash(hsh), height(ht) {

    }

    std::string toJson() const {
        return fmt::format(R"({{"hash": "{}", "height": {}, "time":"2015-06-22T15:58:30Z"}})", hash, height);
    }

};

static std::string mockTransaction(const std::string& hash, const std::vector<Input>& inputs, const std::vector<Output>& outputs, Option<::Block> block = Option<::Block>::NONE) {
    std::ostringstream ss;
    int index = 0;

    ss << fmt::format(R"({{"hash": "{}", "receive_at": "2015-06-22T15:58:30Z", "lock_time": 0, )", hash);
    if (block) {
        ss << fmt::format(R"("block": {}, )", block.getValue().toJson());
    }
    ss << R"("inputs": [)";
    std::vector<std::string> jsonInputs;
    std::transform(inputs.begin(), inputs.end(), std::back_inserter(jsonInputs), [&] (const Input& i) {
        return i.toJson(index++);
    });
    ss << boost::algorithm::join(jsonInputs, ",");
    ss << R"(], "outputs": [)";
    std::vector<std::string> jsonOutputs;
    index = 0;
    std::transform(outputs.begin(), outputs.end(), std::back_inserter(jsonOutputs), [&] (const Output& o) {
        return o.toJson(index++);
    });
    ss << boost::algorithm::join(jsonOutputs, ",");
    ss << R"(], "fees": 100})";
    return ss.str();
}

TEST_F(BitcoinLikeWalletBtcRbfSynchronization, SimpleRbfScenario) {
    auto pool = newPool();
    auto wallet = wait(pool->createWallet("e857815f-488a-4301-b67c-378a5e9c8a63", "bitcoin",
                                          api::DynamicObject::newInstance()));
    auto account = createBitcoinLikeAccount(wallet, 0, P2PKH_MEDIUM_XPUB_INFO);

    ::Block block1 {"0000", 1};
    ::Block block2 {"0001", 2};
    ::Block block3 {"0002", 3};

    Output genesis {500000000, "1KMbwcH1sGpHetLwwQVNMt4cEZB5u8Uk4b" };
    Output send1 {100000000, "1KMbwcH1sGpHetLwwQVNMt4cEZB5u8Uk4b" };
    Output send2 {100000000, "1MKbwcH1sGpHetLwwQVNMt4cEZB5u8Uk4b" };
    Output receive1 {3000000, "1DDBzjLyAmDr4qLRC2T2WJ831cxBM5v7G7" };
    Output receive2 {2000000, "1DDBzjLyAmDr4qLRC2T2WJ831cxBM5v7G7" };
    Output receive3 {3000000, "1DDBzjLyAmDr4qLRC2T2WJ831cxBM5v7G7" };
    Output foreign {3000000, "1Bmpme646SNGa1jjjYAfuijdyBNJXLGEh" };

    auto tx1 = mockTransaction("0001", {Input("0000", 0, genesis)}, {receive1}, block1);
    auto rbfTx1 = mockTransaction("0002", {Input("1000", 0, send1, 0xFFFFFFFB)}, {receive2});
    auto rbfTx2 = mockTransaction("0003", {Input("1001", 1, send2, 0xFFFFFFFA)}, {receive3});

    auto replaceTx1 = mockTransaction("0004", {Input("1000", 0, send1, 0xFFFFFFFC)}, {receive1});
    auto replaceTx2 = mockTransaction("0005", {Input("1001", 1, send2, 0xFFFFFFFB)}, {foreign});

    // Receive a single transaction
    explorer->addTransaction(tx1);
    synchronizeAccount(account);
    EXPECT_EQ(uv::wait(account->getBalance())->toLong(), receive1.value);
    EXPECT_EQ(getAllOperartions(account).size(), 1);

    // Receive 2 RBF transaction
    explorer->addTransaction(rbfTx1);
    explorer->addTransaction(rbfTx2);
    synchronizeAccount(account);
    EXPECT_EQ(uv::wait(account->getBalance())->toLong(), receive1.value + receive2.value + receive3.value);
    EXPECT_EQ(getAllOperartions(account).size(), 3);

    // Replace 1 transaction with another one to the same account
    explorer->addTransaction(replaceTx1);
    synchronizeAccount(account);
    EXPECT_EQ(uv::wait(account->getBalance())->toLong(), receive1.value + receive1.value + receive3.value);
    EXPECT_EQ(getAllOperartions(account).size(), 3);

    // Replace the second transaction with another one on an unrelated account
    explorer->addTransaction(replaceTx2);
    synchronizeAccount(account);
    EXPECT_EQ(uv::wait(account->getBalance())->toLong(), receive1.value + receive1.value);
    EXPECT_EQ(getAllOperartions(account).size(), 2);
}
