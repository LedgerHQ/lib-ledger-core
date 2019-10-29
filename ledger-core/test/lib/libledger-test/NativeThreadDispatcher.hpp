/*
 *
 * NativeThreadDispatcher
 * ledger-core
 *
 * Created by Pierre Pollastri on 21/11/2016.
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

#pragma once

#include <core/api/ThreadDispatcher.hpp>
#include <core/api/ExecutionContext.hpp>
#include <core/api/Runnable.hpp>
#include <core/api/Lock.hpp>

#include "EventThread.hpp"
#include "EventLooper.hpp"

class NativeThreadDispatcher : public ledger::core::api::ThreadDispatcher {
public:
    NativeThreadDispatcher();
    virtual std::shared_ptr<ledger::core::api::ExecutionContext>
    getSerialExecutionContext(const std::string &name) override;

    virtual std::shared_ptr<ledger::core::api::ExecutionContext>
    getThreadPoolExecutionContext(const std::string &name) override;

    virtual std::shared_ptr<ledger::core::api::ExecutionContext> getMainExecutionContext() override;

    virtual std::shared_ptr<ledger::core::api::Lock> newLock() override;

    void stop();
    void waitUntilStopped();

private:
    std::vector<std::pair<std::string, std::shared_ptr<ledger::core::api::ExecutionContext>>> _contexts;

};

std::shared_ptr<ledger::core::api::Runnable> make_runnable(std::function<void()> func);

#define WAIT_AND_TIMEOUT(dispatcher, time)  dispatcher->getSerialExecutionContext("toto")->delay(::make_runnable([dispatcher]() { \
        dispatcher->stop(); \
        FAIL() << "Timeout"; \
    }), time); \
    dispatcher->waitUntilStopped();
