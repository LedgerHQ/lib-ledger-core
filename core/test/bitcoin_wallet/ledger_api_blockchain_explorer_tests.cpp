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
    auto client = std::make_shared<AsioHttpClient>(dispatcher->getSerialExecutionContext("client"));
    auto worker = dispatcher->getSerialExecutionContext("worker");
    auto http = std::make_shared<HttpClient>("https://api.ledgerwallet.com", client, worker);
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
        std::cout << session.getFailure().getMessage() << std::endl;
        EXPECT_EQ(((std::string *)session.getValue())->size(), 36);
        dispatcher->stop();
    });

    dispatcher->waitUntilStopped();
    resolver->clean();
}