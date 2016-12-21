/*
 *
 * PoolTestCaseBootstraper
 * ledger-core
 *
 * Created by Pierre Pollastri on 21/12/2016.
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
#include "PoolTestCaseBootstraper.hpp"
#include <ledger/core/api/WalletPoolBuildCallback.hpp>
#include <ledger/core/api/WalletPoolBuilder.hpp>

class PoolBuilderCallback : public ledger::core::api::WalletPoolBuildCallback {
public:
    PoolBuilderCallback(std::function<void(std::shared_ptr<ledger::core::api::WalletPool>)> callback) {
        _callback = callback;
    }
    virtual void onWalletPoolBuilt(const std::shared_ptr<ledger::core::api::WalletPool> &pool) override {
        _callback(pool);
    }
private:
    std::function<void(std::shared_ptr<ledger::core::api::WalletPool>)> _callback;
};

PoolTestCaseBootstraper::PoolTestCaseBootstraper(const std::string &poolName) {
    _poolName = poolName;
    dispatcher = std::make_shared<NativeThreadDispatcher>();
    resolver = std::make_shared<NativePathResolver>();
    printer = std::make_shared<CoutLogPrinter>(dispatcher->getMainExecutionContext());
    client = std::make_shared<MongooseHttpClient>(dispatcher->getSerialExecutionContext("http"));
}

void PoolTestCaseBootstraper::setup(std::function<void(std::shared_ptr<ledger::core::api::WalletPool>)> callback) {
    auto c = std::make_shared<PoolBuilderCallback>(callback);
    ledger::core::api::WalletPoolBuilder::createInstance()
            ->setThreadDispatcher(dispatcher)
            ->build(c);
}

void PoolTestCaseBootstraper::tearDown() {

}
