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
#include "NativeThreadDispatcher.hpp"

class MainThreadExecutionContext : public ledger::core::api::ExecutionContext {
public:
    MainThreadExecutionContext() {

    }

    virtual void execute(const std::shared_ptr<ledger::core::api::Runnable> &runnable) override {
        std::shared_ptr<ledger::core::api::Runnable> r = runnable;
        _looper.push_back(new LambdaEvent([r] () {
           r->run();
        }));
    }

    virtual void delay(const std::shared_ptr<ledger::core::api::Runnable> &runnable, int64_t millis) override {
        std::shared_ptr<ledger::core::api::Runnable> r = runnable;
        _looper.push_back(new LambdaEvent([r] () {
            r->run();
        }), millis);
    }

    EventLooper* getLooper() {
        return &_looper;
    }

private:
    EventLooper _looper;
};

class SerialQueueExecutionContext : public ledger::core::api::ExecutionContext {
public:
    SerialQueueExecutionContext() {
        _thread.start();
    }
    virtual void execute(const std::shared_ptr<ledger::core::api::Runnable> &runnable) override {
        std::shared_ptr<ledger::core::api::Runnable> r = runnable;
        _thread.getLooper()->push_back(new LambdaEvent([r] () {
            r->run();
        }));
    }

    virtual void delay(const std::shared_ptr<ledger::core::api::Runnable> &runnable, int64_t millis) override {
        std::shared_ptr<ledger::core::api::Runnable> r = runnable;
        _thread.getLooper()->push_back(new LambdaEvent([r] () {
            r->run();
        }), millis);
    }

private:
    EventThread _thread;
};

class MutexLock : public ledger::core::api::Lock {
public:
    virtual void lock() override {
        _mutex.lock();
    }

    virtual bool tryLock() override {
        return _mutex.try_lock();
    }

    virtual void unlock() override {
        _mutex.unlock();
    }

private:
    std::recursive_mutex _mutex;
};

NativeThreadDispatcher::NativeThreadDispatcher() {
    _contexts.push_back(std::pair<std::string, std::shared_ptr<ledger::core::api::ExecutionContext>>("___main___", std::make_shared<MainThreadExecutionContext>()));
}

std::shared_ptr<ledger::core::api::ExecutionContext>
NativeThreadDispatcher::getSerialExecutionContext(const std::string &name) {
    for (auto& ctx : _contexts) {
        if (ctx.first == name) {
            return ctx.second;
        }
    }
    _contexts.push_back(std::pair<std::string, std::shared_ptr<ledger::core::api::ExecutionContext>>(name, std::make_shared<SerialQueueExecutionContext>()));
    return _contexts.back().second;
}

std::shared_ptr<ledger::core::api::ExecutionContext>
NativeThreadDispatcher::getThreadPoolExecutionContext(const std::string &name) {
    return getSerialExecutionContext(name); // For testing purpose, thread pool execution contexts are just serial queues.
}

std::shared_ptr<ledger::core::api::ExecutionContext> NativeThreadDispatcher::getMainExecutionContext() {
    return _contexts.front().second;
}

std::shared_ptr<ledger::core::api::Lock> NativeThreadDispatcher::newLock() {
    return std::make_shared<MutexLock>();
}

void NativeThreadDispatcher::stop() {
    auto looper = std::dynamic_pointer_cast<MainThreadExecutionContext>(_contexts.front().second)->getLooper();
    looper->push_back(new LambdaEvent([looper] () {
        looper->stop();
    }));
}

void NativeThreadDispatcher::waitUntilStopped() {
    std::dynamic_pointer_cast<MainThreadExecutionContext>(_contexts.front().second)->getLooper()->run();
}

std::shared_ptr<ledger::core::api::Runnable> make_runnable(std::function<void()> func) {
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