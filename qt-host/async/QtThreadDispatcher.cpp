/*
 *
 * QtThreadDispatcher
 * ledger-core
 *
 * Created by Pierre Pollastri on 19/04/2017.
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
#include "QtThreadDispatcher.hpp"
#include "QtThreadPoolExecutionContext.hpp"
#include "QtMainExecutionContext.hpp"

std::shared_ptr<ledger::core::api::ExecutionContext>
ledger::qt::QtThreadDispatcher::getSerialExecutionContext(const std::string &name) {
    QMutexLocker locker(&_mutex);
    auto serialQueueName = name + "_";
    if (!_contexts.contains(serialQueueName)) {
       _contexts[serialQueueName] = std::make_shared<QtThreadPoolExecutionContext>(1, getMainExecutionContext());
    }
    return _contexts[serialQueueName];
}

std::shared_ptr<ledger::core::api::ExecutionContext>
ledger::qt::QtThreadDispatcher::getThreadPoolExecutionContext(const std::string &name) {
    QMutexLocker locker(&_mutex);
    auto contextName = name + "$";
    if (!_contexts.contains(contextName)) {
       _contexts[contextName] = std::make_shared<QtThreadPoolExecutionContext>(_maxThreads, getMainExecutionContext());
    }
    return _contexts[contextName];
}

std::shared_ptr<ledger::core::api::Lock> ledger::qt::QtThreadDispatcher::newLock() {
    return nullptr;
}

ledger::qt::QtThreadDispatcher::QtThreadDispatcher(std::shared_ptr<QCoreApplication> app) {
    _maxThreads = QThread::idealThreadCount();
    if (app == nullptr) {
        _contexts["main"] = std::make_shared<QtMainExecutionContext>();
    } else {
        _contexts["main"] = std::make_shared<QtMainExecutionContext>(app);
    }
}

void ledger::qt::QtThreadDispatcher::setMaximumThreadPoolThreads(int num) {
    _maxThreads = num;
}

std::shared_ptr<ledger::core::api::ExecutionContext> ledger::qt::QtThreadDispatcher::getMainExecutionContext() {
    return _contexts["main"];
}

void ledger::qt::QtThreadDispatcher::stop() {
    std::static_pointer_cast<QtMainExecutionContext>(getMainExecutionContext())->exit();
}

int ledger::qt::QtThreadDispatcher::waitUntilStopped() {
    return std::static_pointer_cast<QtMainExecutionContext>(getMainExecutionContext())->run();
}

std::shared_ptr<ledger::core::api::Runnable> ledger::qt::make_runnable(std::function<void()> func) {
    class LambdaRunnable : public ledger::core::api::Runnable {
    public:
        LambdaRunnable(std::function<void()> func) {
            _func = func;
        }

        virtual void run() override {
            _func();
        }
    private:
        std::function<void()> _func;
    };

    return std::make_shared<LambdaRunnable>(func);
}