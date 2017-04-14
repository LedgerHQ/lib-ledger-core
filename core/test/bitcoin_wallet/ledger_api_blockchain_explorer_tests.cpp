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

TEST(LedgerApiBitcoinLikeBlockchainExplorer, GetTransactionByHash) {
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
    explorer->getTransactionByHash("9fdbe15a16fe282291426df15894ab1473e252bc31f244e4d923a17e11743eda").onComplete(worker, [&] (const Try<BitcoinLikeBlockchainExplorer::Transaction>& transaction) {
        if (transaction.isFailure()) {
            std::cerr << transaction.getFailure().getMessage() << std::endl;
            dispatcher->stop();
            FAIL();
        } else {
            auto &tx = transaction.getValue();
            EXPECT_EQ(tx.inputs.size(), 1);
            EXPECT_EQ(tx.hash, "9fdbe15a16fe282291426df15894ab1473e252bc31f244e4d923a17e11743eda");
            EXPECT_EQ(tx.inputs[0].value.getValue().toString(), "1634001");
            EXPECT_EQ(tx.inputs[0].address.getValue(), "1Nd2kJid5fFmPks9KSRpoHQX4VpkPhuATm");
            EXPECT_EQ(tx.fees.getValue().toString(), "11350");
            EXPECT_EQ(tx.outputs.size(), 2);
            EXPECT_EQ(tx.outputs[0].value.toString(), "1000");
            EXPECT_EQ(tx.outputs[1].value.toString(), "1621651");
            EXPECT_EQ(tx.outputs[0].address.getValue(), "1QKJghDW4kLqCsH2pq3XKKsSSeYNPcL5PD");
            EXPECT_EQ(tx.outputs[1].address.getValue(), "19j8biFtMSy5HFRX6mXiurjz3jszg7nLN5");
            EXPECT_EQ(tx.block.getValue().hash, "0000000000000000026aa418ef33e0b079a42d348f35bc0a2fa4bc150a9c459d");
            EXPECT_EQ(tx.block.getValue().height, 403912);
            EXPECT_EQ(tx.block.getValue().time.time_since_epoch().count(), 1458734061000000);
            EXPECT_EQ(tx.receivedAt.time_since_epoch().count(), 1458734061000000);
        }
        dispatcher->stop();
    });

    dispatcher->waitUntilStopped();
    resolver->clean();
}

TEST(LedgerApiBitcoinLikeBlockchainExplorer, GetTransactionByHash_2) {
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
    explorer->getTransactionByHash("16da85a108a63ff318458be597f34f0a7f6b9f703528249056ba2f48722ae44e").onComplete(worker, [&] (const Try<BitcoinLikeBlockchainExplorer::Transaction>& transaction) {
        if (transaction.isFailure()) {
            std::cerr << transaction.getFailure().getMessage() << std::endl;
            dispatcher->stop();
            FAIL();
        } else {
            auto &tx = transaction.getValue();
            EXPECT_EQ(tx.inputs.size(), 1);
            EXPECT_EQ(tx.hash, "16da85a108a63ff318458be597f34f0a7f6b9f703528249056ba2f48722ae44e");
            EXPECT_EQ(tx.inputs.size(), 1);
            EXPECT_EQ(tx.outputs.size(), 1);
            EXPECT_EQ(tx.inputs[0].coinbase.getValue(), "03070c070004ebabf05804496e151608bef5342d8b2800000a425720537570706f727420384d200a666973686572206a696e78696e092f425720506f6f6c2f");
            EXPECT_EQ(tx.outputs[0].address.getValue(), "1BQLNJtMDKmMZ4PyqVFfRuBNvoGhjigBKF");
            EXPECT_EQ(tx.outputs[0].value.toString(), "1380320309");
        }
        dispatcher->stop();
    });

    dispatcher->waitUntilStopped();
    resolver->clean();
}

TEST(LedgerApiBitcoinLikeBlockchainExplorer, GetTransactionByHash_3) {
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
    explorer->getTransactionByHash("8d2a0ccbe3a71f3e505be1557995c57f2a26f1951a72931f23a61f18fa4b3d2d").onComplete(worker, [&] (const Try<BitcoinLikeBlockchainExplorer::Transaction>& transaction) {
        if (transaction.isFailure()) {
            std::cerr << transaction.getFailure().getMessage() << std::endl;
            dispatcher->stop();
            FAIL();
        } else {
            auto &tx = transaction.getValue();
            EXPECT_EQ(tx.hash, "8d2a0ccbe3a71f3e505be1557995c57f2a26f1951a72931f23a61f18fa4b3d2d");
            EXPECT_EQ(tx.inputs.size(), 8);
            EXPECT_EQ(tx.outputs.size(), 2);
            EXPECT_EQ(tx.inputs[5].value.getValue().toString(), "270000");
            EXPECT_EQ(tx.inputs[5].index, 5);
            EXPECT_EQ(tx.inputs[5].address.getValue(), "1BEG75jXGZgH7QsSNjmm9RGJ2fgWcXVbxm");
            EXPECT_EQ(tx.inputs[5].signatureScript.getValue(), "483045022100b21b21023b15be3d71fc660513adc4ef1aaa299ee58b9a5c1b8401015d045622022031847f047494c83b199a743d5edd5dbeb33b2dae03dcdff12485b212061d0463012102a7e1245393aa50cf6e08077ac5f4460c2db9c54858f6b0958d91b8d62f39c3bb");
            EXPECT_EQ(tx.inputs[5].previousTxHash.getValue(), "64717373eef15249771032b0153daae92d18ea63e997c1c70a33879698b43329");
            EXPECT_EQ(tx.inputs[5].previousTxOutputIndex.getValue(), 9);
            EXPECT_EQ(tx.outputs[0].address.getValue(), "14w1wdDMV5uSnBd92yf3N9LfgS6TKVzyYr");
            EXPECT_EQ(tx.outputs[1].address.getValue(), "1pCL4HJ3wbNXKiDde8eNmu9uMs1Tkd9hD");
        }
        dispatcher->stop();
    });

    dispatcher->waitUntilStopped();
    resolver->clean();
}