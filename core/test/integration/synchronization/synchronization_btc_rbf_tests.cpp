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
                           std::cout << api::to_string(event->getCode()) << std::endl;
                           if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                               return;
                           promise.success(unit);
                       }));
        uv::wait(promise.getFuture());
    }

    std::shared_ptr<test::ExplorerStorage> explorer;
};

struct Output {
    //   [{"output_index":0,"value":182483500,"address":"18BkSm7P2wQJfQhV7B5st14t13mzHRJ2o1","script_hex":"76a9144ed14e321713e1c97056643a233b968d34b2231188ac"},{"output_index":1,"value":100000,"address":"1NMfmPC9yHBe5US2CUwWARPRM6cDP6N86m","script_hex":"76a914ea4351fd2a0a2cd62ab264d9f0b1997696a632f488ac"}]
    uint64_t value;
    std::string address;

    Output(uint64_t val, const std::string& addr) : value(val), address(addr) {}

    std::string toJson(int index) {
        return fmt::format(R"({{"output_index": {}, "address": "{}", "value": {}, "script_hex": "0000"}})", index, address, value);
    }
};

struct Input {
// {"input_index":0,"output_hash":"666613fd82459f94c74211974e74ffcb4a4b96b62980a6ecaee16af7702bbbe5","output_index":0,"value":182593500,"address":"1DDBzjLyAmDr4qLRC2T2WJ831cxBM5v7G7","script_signature":"473044022018ac8a44412d66f489138c3e8f9196b60dba1c24fb715dd8c3d66921bcc13f4702201f222fd3e25fe8f3807347650ae7b41451c078c9a8fc2e5b7d82d6a928a8c363012103da611b1fcacc0056ceb5489ee951b523e52de7ff1902fd1c6f9c212a542da297"}]
    std::string outputHash;
    int outputIndex;
    Output prevOutput;
    uint64_t sequence;

    Input(const std::string& hash, int index, const Output& output, uint64_t seq)
        : outputHash(hash), outputIndex(index), prevOutput(output), sequence(seq) {

    }

};

struct Block {
    // {"hash":"00000000000000000198e024d936c87807b4198f9de7105015036ce785fa2bdc","height":362055,"time":"2015-06-22T15:58:30Z"}
};

static std::string mockTransaction(const std::vector<Input>& input, const std::vector<Output>& output) {
    std::ostringstream ss;

    //"{\"hash\":\"\",\"received_at\":\"2015-06-22T15:58:30Z\",\"lock_time\":0,\"block\":,\"inputs\":,\"outputs\":,\"fees\":,\"amount\":0,\"confirmations\":110200}"
    return "";
}

Future<int> do_it(std::shared_ptr<api::ExecutionContext> context, int n, int max) {
    return Future<int>::async(context, [=] () {
        std::cout << " -> " <<  n << std::endl;
        return n + 1;
    }).flatMap<int>(context, [=] (int v) {
        if (v < max) {
            return do_it(context, v, max);
        }
        return Future<int>::successful(v);
    });
}

TEST_F(BitcoinLikeWalletBtcRbfSynchronization, SimpleRbfScenario) {
    auto pool = newPool();
    auto wallet = wait(pool->createWallet("e857815f-488a-4301-b67c-378a5e9c8a63", "bitcoin",
                                          api::DynamicObject::newInstance()));
    auto account = createBitcoinLikeAccount(wallet, 0, P2PKH_MEDIUM_XPUB_INFO);
    synchronizeAccount(account);
    synchronizeAccount(account);

    std::cout <<  Output(9000, "1DDBzjLyAmDr4qLRC2T2WJ831cxBM5v7G7").toJson(12) << std::endl;

}