/*
 *
 * promise_test
 * ledger-core
 *
 * Created by Pierre Pollastri on 24/01/2017.
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

#include <core/async/Promise.hpp>

#include <NativeThreadDispatcher.hpp>

using namespace ledger::core;

TEST(Promise, Success) {
    auto dispatcher = std::make_shared<NativeThreadDispatcher>();
    auto queue = dispatcher->getSerialExecutionContext("queue");

    Promise<std::string> promise;

    promise.getFuture().foreach(queue, [dispatcher] (const std::string& result) {
        EXPECT_EQ("Hello world", result);
        dispatcher->stop();
    });

    promise.success("Hello world");
    dispatcher->waitUntilStopped();
}

TEST(Promise, Failure) {
    auto dispatcher = std::make_shared<NativeThreadDispatcher>();
    auto queue = dispatcher->getSerialExecutionContext("queue");

    Promise<std::string> promise;

    promise.getFuture().failed().foreach(queue, [dispatcher] (const Exception& result) {
        EXPECT_EQ(api::ErrorCode::ILLEGAL_STATE, result.getErrorCode());
        dispatcher->stop();
    });

    promise.failure(Exception(api::ErrorCode::ILLEGAL_STATE, "Nuke"));
    dispatcher->waitUntilStopped();
}

TEST(Promise, Complete) {
    auto dispatcher = std::make_shared<NativeThreadDispatcher>();
    auto queue = dispatcher->getSerialExecutionContext("queue");

    Promise<std::string> promise;

    promise.getFuture().foreach(queue, [dispatcher] (const std::string& result) {
        EXPECT_EQ("Hello world", result);
        dispatcher->stop();
    });

    promise.complete(Try<std::string>("Hello world"));
    dispatcher->waitUntilStopped();
}

TEST(Promise, CompleteWith) {
    auto dispatcher = std::make_shared<NativeThreadDispatcher>();
    auto queue = dispatcher->getSerialExecutionContext("queue");

    Promise<std::string> promise;

    promise.getFuture().foreach(queue, [dispatcher] (const std::string& result) {
        EXPECT_EQ("Hello world", result);
        dispatcher->stop();
    });

    promise.completeWith(Future<std::string>::async(queue, [] () {
        return "Hello world";
    }));
    dispatcher->waitUntilStopped();
}

TEST(Promise, CompleteWithFailure) {
    auto dispatcher = std::make_shared<NativeThreadDispatcher>();
    auto queue = dispatcher->getSerialExecutionContext("queue");

    Promise<std::string> promise;

    promise.getFuture().failed().foreach(queue, [dispatcher] (const Exception& result) {
        EXPECT_EQ(api::ErrorCode::ILLEGAL_STATE, result.getErrorCode());
        dispatcher->stop();
    });

    promise.completeWith(Future<std::string>::async(queue, [] () -> std::string {
        throw Exception(api::ErrorCode::ILLEGAL_STATE, "Nuke");
    }));
    dispatcher->waitUntilStopped();
}

TEST(Promise, TrySuccess) {
    auto dispatcher = std::make_shared<NativeThreadDispatcher>();
    auto queue = dispatcher->getSerialExecutionContext("queue");

    Promise<std::string> promise;

    promise.getFuture().foreach(queue, [dispatcher] (const std::string& result) {
        EXPECT_EQ("Hello world", result);
        dispatcher->stop();
    });

    promise.success("Hello world");
    EXPECT_THROW(promise.success("Hello Toto"), Exception);
    dispatcher->waitUntilStopped();
}
