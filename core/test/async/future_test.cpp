/*
 *
 * future_test
 * ledger-core
 *
 * Created by Pierre Pollastri on 23/01/2017.
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
#include <ledger/core/async/Future.hpp>
#include <NativeThreadDispatcher.hpp>
#include <iostream>

using namespace ledger::core;

TEST(Future, OnCompleteSuccess) {
    auto dispatcher = std::make_shared<NativeThreadDispatcher>();
    auto queue = dispatcher->getSerialExecutionContext("queue");
    Future<std::string>::async(queue, [] () -> std::string {
        return "Hello world";
    }).onComplete(dispatcher->getMainExecutionContext(), [dispatcher] (const Try<std::string>& result) {
        EXPECT_TRUE(result.isSuccess());
        if (result.isSuccess()) {
            EXPECT_EQ("Hello world", result.getValue());
        }
        dispatcher->stop();
    });
    dispatcher->waitUntilStopped();
}

TEST(Future, OnCompleteFailure) {
    auto dispatcher = std::make_shared<NativeThreadDispatcher>();
    auto queue = dispatcher->getSerialExecutionContext("queue");
    Future<std::string>::async(queue, [] () -> std::string {
       throw std::out_of_range("Not good");
    }).onComplete(dispatcher->getMainExecutionContext(), [dispatcher] (const Try<std::string>& result) {
        EXPECT_TRUE(result.isFailure());
        if (result.isFailure()) {
            EXPECT_EQ(api::ErrorCode::RUNTIME_ERROR, result.getFailure().getErrorCode());
        }
        dispatcher->stop();
    });
    dispatcher->waitUntilStopped();
}

TEST(Future, Map) {
    auto dispatcher = std::make_shared<NativeThreadDispatcher>();
    auto queue = dispatcher->getSerialExecutionContext("queue");
    Future<std::string>::async(queue, [] () -> std::string {
        return "Hello world";
    }).map<std::string>(queue, [] (const std::string& str) -> std::string {
        return str + " from another thread";
    }).foreach(dispatcher->getMainExecutionContext(), [dispatcher] (const std::string& value) {
        EXPECT_EQ("Hello world from another thread", value);
        dispatcher->stop();
    });

    dispatcher->waitUntilStopped();
}

TEST(Future, MapToInt) {
    auto dispatcher = std::make_shared<NativeThreadDispatcher>();
    auto queue = dispatcher->getSerialExecutionContext("queue");
    Future<std::string>::async(queue, [] () -> std::string {
        return "42";
    }).map<int>(queue, [] (const std::string& str) -> int {
        return std::atoi(str.c_str());
    }).foreach(dispatcher->getMainExecutionContext(), [dispatcher] (const int& value) {
        EXPECT_EQ(42, value);
        dispatcher->stop();
    });

    dispatcher->waitUntilStopped();
}

TEST(Future, FlatMap) {
    auto dispatcher = std::make_shared<NativeThreadDispatcher>();
    auto queue = dispatcher->getSerialExecutionContext("queue");
    Future<std::string>::async(queue, [] () -> std::string {
        return "Hello world";
    }).flatMap<std::string>(queue, [queue] (const std::string& str) {
        return Future<std::string>::async(queue, [str] () {
            return str + " from another thread";
        });
    }).foreach(dispatcher->getMainExecutionContext(), [dispatcher] (const std::string& value) {
        EXPECT_EQ("Hello world from another thread", value);
        dispatcher->stop();
    });
    dispatcher->waitUntilStopped();
}

TEST(Future, MapChain) {
    auto dispatcher = std::make_shared<NativeThreadDispatcher>();
    auto queue = dispatcher->getSerialExecutionContext("queue");
    Future<std::string>::async(queue, [] () -> std::string {
        return "Hello world";
    }).map<std::string>(queue, [] (const std::string& str) -> std::string {
        return str + " from";
    }).map<std::string>(queue, [] (const std::string& str) -> std::string {
        return str + " another";
    }).map<std::string>(queue, [] (const std::string& str) -> std::string {
        return str + " thread";
    }).foreach(dispatcher->getMainExecutionContext(), [dispatcher] (const std::string& value) {
        EXPECT_EQ("Hello world from another thread", value);
        dispatcher->stop();
    });
    dispatcher->waitUntilStopped();
}

TEST(Future, MapChainRecover) {
    auto dispatcher = std::make_shared<NativeThreadDispatcher>();
    auto queue = dispatcher->getSerialExecutionContext("queue");
    Future<std::string>::async(queue, [] () -> std::string {
        return "Hello world";
    }).map<std::string>(queue, [] (const std::string& str) -> std::string {
        return str + " from";
    }).map<std::string>(queue, [] (const std::string& str) -> std::string {
        return str + " another";
    }).map<std::string>(queue, [] (const std::string& str) -> std::string {
        throw std::out_of_range("Not good");
    }).recover(queue, [] (const Exception& ex) {
        return "Whew, we recovered !";
    }).foreach(dispatcher->getMainExecutionContext(), [dispatcher] (const std::string& value) {
        EXPECT_EQ("Whew, we recovered !", value);
        dispatcher->stop();
    });
    dispatcher->waitUntilStopped();
}

TEST(Future, MapChainRecoverWith) {
    auto dispatcher = std::make_shared<NativeThreadDispatcher>();
    auto queue = dispatcher->getSerialExecutionContext("queue");
    Future<std::string>::async(queue, [] () -> std::string {
        return "Hello world";
    }).map<std::string>(queue, [] (const std::string& str) -> std::string {
        return str + " from";
    }).map<std::string>(queue, [] (const std::string& str) -> std::string {
        return str + " another";
    }).map<std::string>(queue, [] (const std::string& str) -> std::string {
        throw std::out_of_range("Not good");
    }).recoverWith(queue, [queue] (const Exception& ex) {
        return Future<std::string>::async(queue, [] () {
            return "Whew, we recovered !";
        });
    }).foreach(dispatcher->getMainExecutionContext(), [dispatcher] (const std::string& value) {
        EXPECT_EQ("Whew, we recovered !", value);
        dispatcher->stop();
    });
    dispatcher->waitUntilStopped();
}

TEST(Future, MapChainRecoverWithFail) {
    auto dispatcher = std::make_shared<NativeThreadDispatcher>();
    auto queue = dispatcher->getSerialExecutionContext("queue");
    Future<std::string>::async(queue, [] () -> std::string {
        return "Hello world";
    }).map<std::string>(queue, [] (const std::string& str) -> std::string {
        return str + " from";
    }).map<std::string>(queue, [] (const std::string& str) -> std::string {
        return str + " another";
    }).map<std::string>(queue, [] (const std::string& str) -> std::string {
        throw std::out_of_range("Not good");
    }).recoverWith(queue, [queue] (const Exception& ex) {
        return Future<std::string>::async(queue, [] () -> std::string{
            throw std::out_of_range("Not good");
        });
    }).onComplete(dispatcher->getMainExecutionContext(), [dispatcher] (const Try<std::string>& result) {
        EXPECT_TRUE(result.isFailure());
        dispatcher->stop();
    });
    dispatcher->waitUntilStopped();
}

TEST(Future, FailureProjection) {
    auto dispatcher = std::make_shared<NativeThreadDispatcher>();
    auto queue = dispatcher->getSerialExecutionContext("queue");
    Future<std::string>::async(queue, [] () -> std::string {
        throw Exception(api::ErrorCode::ILLEGAL_STATE, "We shouldn't do that");
    }).failed().foreach(dispatcher->getMainExecutionContext(), [dispatcher] (const Exception& ex) {
        EXPECT_EQ(api::ErrorCode::ILLEGAL_STATE, ex.getErrorCode());
        dispatcher->stop();
    });
    dispatcher->waitUntilStopped();
}