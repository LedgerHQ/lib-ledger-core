/*
 *
 * ledger_api_blockchain_explorer_tests
 * ledger-core
 *
 * Created by Pierre Pollastri on 10/03/2017.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Ledger
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
#include <ledger/core/wallet/bitcoin/explorers/LedgerApiBitcoinLikeBlockchainExplorer.hpp>
#include <AsioHttpClient.hpp>
#include <ledger/core/wallet/bitcoin/networks.hpp>
#include <NativeThreadDispatcher.hpp>
#include <CoutLogPrinter.hpp>
#include <NativePathResolver.hpp>
#include <MongooseHttpClient.hpp>
#include <ledger/core/utils/hex.h>

using namespace ledger::core;

TEST(LedgerApiBitcoinLikeBlockchainExplorer, StartSession) {
    auto dispatcher = std::make_shared<NativeThreadDispatcher>();
    auto client = std::make_shared<MongooseHttpClient>(dispatcher->getSerialExecutionContext("client"));
    auto worker = dispatcher->getSerialExecutionContext("worker");
    auto http = std::make_shared<HttpClient>("http://api.ledgerwallet.com", client, worker);
    auto explorer = std::make_shared<LedgerApiBitcoinLikeBlockchainExplorer>(worker, http, networks::BITCOIN);
    auto logPrinter = std::make_shared<CoutLogPrinter>(dispatcher->getMainExecutionContext());
    auto resolver = std::make_shared<NativePathResolver>();
    auto logger = ledger::core::logger::create("test_logs",
                                                                          std::experimental::optional<std::string>(),
                                                                          dispatcher->getSerialExecutionContext("logger"),
                                                                          resolver,
                                                                          logPrinter,
                                                                          (std::string("2017-03-02T10:07:06Z+01:00 D: This is a log \n").size() + 3) * 199
    );
    http->setLogger(logger);
    explorer->startSession().onComplete(worker, [&] (const Try<void *>& session) {
        EXPECT_TRUE(session.isSuccess());
        EXPECT_EQ(((std::string *)session.getValue())->size(), 36);
        dispatcher->stop();
    });

    dispatcher->waitUntilStopped();
    resolver->clean();
}

TEST(LedgerApiBitcoinLikeBlockchainExplorer, GetRawTransaction) {
    auto dispatcher = std::make_shared<NativeThreadDispatcher>();
    auto client = std::make_shared<MongooseHttpClient>(dispatcher->getSerialExecutionContext("client"));
    auto worker = dispatcher->getSerialExecutionContext("worker");
    auto http = std::make_shared<HttpClient>("http://api.ledgerwallet.com", client, worker);
    auto explorer = std::make_shared<LedgerApiBitcoinLikeBlockchainExplorer>(worker, http, networks::BITCOIN);
    auto logPrinter = std::make_shared<CoutLogPrinter>(dispatcher->getMainExecutionContext());
    auto resolver = std::make_shared<NativePathResolver>();
    auto logger = ledger::core::logger::create("test_logs",
                                               std::experimental::optional<std::string>(),
                                               dispatcher->getSerialExecutionContext("logger"),
                                               resolver,
                                               logPrinter,
                                               (std::string("2017-03-02T10:07:06Z+01:00 D: This is a log \n").size() + 3) * 199
    );
    http->setLogger(logger);
    explorer->getRawTransaction("9d7945129b78e2f63a72fed93e8ebe38567bdc9318591cfe8c8a7de76c5cb1a3").onComplete(worker, [&] (const Try<Bytes>& transaction) {
        EXPECT_TRUE(transaction.isSuccess());
        auto hex = transaction.getValue().toHex();
        EXPECT_EQ(hex.str(), "0100000001d62dad27a2bdd0c5e72a6288acb4e0acac088b4bc5588e60ff5c3861c4584d71010000006b483045022100d72a8e43c74764a18c5dfec225f1e60dceb12a9bf4931afa1093f14c471f55d202202cf4ed0956fd68dc9ba9d026a4ae04758092487cebff1618e320dcc12d736577012102b62b6c66c0d69ca3272ed3d0884a40bd4fb50ab08bec6de6d899b7389f40e9b5ffffffff026fa40200000000001976a91459fa62dab1f04b4528e5c5446f4c897b53fc983c88ace58f8b00000000001976a914b026e605bb239cf7eafb6437667f0f7f80e827f488ac00000000");
        dispatcher->stop();
    });

    dispatcher->waitUntilStopped();
    resolver->clean();
}