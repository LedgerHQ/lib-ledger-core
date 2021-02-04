/*
 *
 * UvThreadDispatcher.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 14/06/2020.
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

#include "UvThreadDispatcher.hpp"
#include <thread>
#include <ledger/core/utils/Option.hpp>
#include <ledger/core/utils/Try.hpp>
#include <unordered_map>

using namespace ledger::core;

namespace uv {

    static void timer_callback(uv_timer_t* handle) {
        auto event = static_cast<Event*>(handle->data);
        event->runnable->run();
        delete event;
    }

    static void context_callback(uv_async_t* handle) {
        static_cast<EventLoop *>(handle->data)->tick();
    }

    //////////////// EventLoop //////////////////////

    EventLoop::EventLoop() : 
    _loop(new uv_loop_t), _async(new uv_async_t), _timer(new uv_timer_t), _running(true) {
        uv_loop_init(_loop);
        uv_async_init(_loop, _async, context_callback);
        _async->data = static_cast<void *>(this);
        uv_timer_init(_loop, _timer);
    }

    void EventLoop::queue(const std::shared_ptr<ledger::core::api::Runnable>& runnable) {
        queue(runnable, 0);
    }

    void EventLoop::queue(const std::shared_ptr<ledger::core::api::Runnable>& runnable, int64_t millis) {
        std::unique_lock<std::mutex> lock(_mutex);
        if (_running) {
            Event event;
            event.eventType = EventType::RUN;
            event.runnable = runnable;
            event.delay = millis;
            _queue.emplace(std::move(event));
            uv_async_send(_async);
        }
        else {
            std::cout << "Loop queue not running" << std::endl;
        }
    }

    bool EventLoop::run() {
        uv_run(_loop, UV_RUN_DEFAULT);
        return uv_loop_alive(_loop) != 0;
    }

    void EventLoop::tick() {
        ledger::core::Option<Event> event;
        while ((event = dequeue()).hasValue()) {
            switch (event.getValue().eventType) {
                case EventType::RUN: {
                    if (event.getValue().delay == 0) {
                        auto result = ledger::core::Try<ledger::core::Unit>::from([&]() {
                            event.getValue().runnable->run();
                            return ledger::core::unit;
                        });
                        if (result.isFailure()) {
                            std::cout << "An error happened during asynchronous execution: "
                                    << result.getFailure().getMessage() << std::endl;
                        }
                    }
                    else {
                        auto evt = new Event(event.getValue());
                        _timer->data = static_cast<void *>(evt);
                        uv_timer_start(_timer, timer_callback, event.getValue().delay, 0);
                    }
                    break;
                }
                case EventType::CLOSE: {
                    std::unique_lock<std::mutex> lock(_mutex);
                    _running = false;
                    uv_timer_stop(_timer);
                    uv_stop(_loop);
                    uv_loop_close(_loop);
                    uv_close(reinterpret_cast<uv_handle_t*>(_async), NULL);
                    lock.unlock();
                    condition.notify_all();
                    break;
                }
            }
        }
    }

    void EventLoop::waitUntilStopped() { 
        std::unique_lock<std::mutex> lock(_mutex);
        condition.wait(lock, [&]{return !_running;});
    }

    void EventLoop::stop() {
        std::unique_lock<std::mutex> lock(_mutex);
        if (_running) {
            Event event;
            event.eventType = EventType::CLOSE;
            _queue.emplace(event);
            uv_async_send(_async);
        }
    }

    ledger::core::Option<Event> EventLoop::dequeue() {
        std::unique_lock<std::mutex> lock(_mutex);
        if (!_queue.empty() && _running) {
            auto event = _queue.front();
            _queue.pop();
            return event;
        } else {
            return ledger::core::Option<Event>::NONE;
        }
    }

    EventLoop::~EventLoop() {
        delete _timer;
        delete _loop;
        delete _async;
    }

    //////////////// SequentialExecutionContext //////////////////////

    SequentialExecutionContext::SequentialExecutionContext() {
        _thread = std::thread([this] () {
            run();
        });
    }

    void SequentialExecutionContext::run() {
        while (_loop.run());
    }

    void SequentialExecutionContext::stop() {
        _loop.stop();
    }

    void SequentialExecutionContext::execute(const std::shared_ptr<ledger::core::api::Runnable> &runnable) {
        _loop.queue(runnable);
    }

    void SequentialExecutionContext::delay(const std::shared_ptr<ledger::core::api::Runnable> &runnable, int64_t millis) {
        _loop.queue(runnable, millis);
    }

    SequentialExecutionContext::~SequentialExecutionContext() {
        _thread.join();
    }

    void SequentialExecutionContext::waitUntilStopped() { 
        _loop.waitUntilStopped();
    }

    //////////////// ThreadpoolExecutionContext //////////////////////

    ThreadpoolExecutionContext::ThreadpoolExecutionContext() : _stop(false) {
        _threads.reserve(THREADPOOL_SIZE);
        for (size_t i = 0; i < THREADPOOL_SIZE; i++)
            _threads.emplace_back(
                [this]
                {
                    for (;;)
                    {
                        std::shared_ptr<ledger::core::api::Runnable> task;
                        {
                            std::unique_lock<std::mutex> lock(this->queue_mutex);
                            this->condition.wait(lock,
                                [this] { return this->_stop || !this->tasks.empty(); });
                            if (this->_stop && this->tasks.empty())
                                return;
                            task = this->tasks.front();
                            this->tasks.pop();
                        }
                        task->run();
                    }
                }
                );
    }

    void ThreadpoolExecutionContext::stop()
    {
        _stop = true;
    }

    void ThreadpoolExecutionContext::execute(const std::shared_ptr<ledger::core::api::Runnable>& runnable) {
        {
            std::unique_lock<std::mutex> lock(this->queue_mutex);
            if (_stop)
                return;
            tasks.emplace(runnable);
        }
        condition.notify_one();
    }

    void ThreadpoolExecutionContext::delay(const std::shared_ptr<ledger::core::api::Runnable>& runnable, int64_t millis) {
        //TODO
    }
    
    ThreadpoolExecutionContext::~ThreadpoolExecutionContext() {
        _stop = true;
        condition.notify_all();
        for (int i = 0; i < THREADPOOL_SIZE; i++)
        {
            if (_threads[i].joinable())
                _threads[i].join();
        }
    }

    //////////////////  UvThreadDispatcher  //////////////////////

    UvThreadDispatcher::UvThreadDispatcher(unsigned int poolSize) : _poolSize(poolSize) {
        _pool.reserve(_poolSize);
    }

    std::shared_ptr<ledger::core::api::ExecutionContext> UvThreadDispatcher::getSerialExecutionContext(const std::string &name) {
        std::unique_lock<std::mutex> lock(_mutex);
        auto it = _contextsByName.find(name);
        if (it == _contextsByName.end()) {
            auto context = std::make_shared<SequentialExecutionContext>();
            _pool.push_back(context);
            _contextsByName[name] = context;
            return context;
        }
        return it->second;
    }

    std::shared_ptr<ledger::core::api::ExecutionContext> UvThreadDispatcher::getThreadPoolExecutionContext(const std::string &name) {
        std::unique_lock<std::mutex> lock(_mutex);
        auto it = _threadpoolContextsByName.find(name);
        if (it == _threadpoolContextsByName.end()) {
            auto context = std::make_shared<ThreadpoolExecutionContext>();
            _threadpoolContextsByName[name] = context;
            return context;
        }
        return it->second;
    }

    std::shared_ptr<ledger::core::api::ExecutionContext> UvThreadDispatcher::getMainExecutionContext() {
        return getSerialExecutionContext("__uv__main__");
    }

    std::shared_ptr<ledger::core::api::Lock> UvThreadDispatcher::newLock() {
        return nullptr;
    }

    UvThreadDispatcher::~UvThreadDispatcher() {
        std::unique_lock<std::mutex> lock(_mutex);
        for (auto& context : _pool) {
            context->stop();
            context->waitUntilStopped();
        }
    }

    void UvThreadDispatcher::waitUntilStopped() {
        auto context = std::dynamic_pointer_cast<SequentialExecutionContext>(
            getSerialExecutionContext("__uv__main__"));
        context->waitUntilStopped();
    }

    void UvThreadDispatcher::stop() {
        auto context = std::dynamic_pointer_cast<SequentialExecutionContext>(
            getSerialExecutionContext("__uv__main__"));
        context->stop();
    }


    std::shared_ptr<ledger::core::api::ThreadDispatcher> createDispatcher() {
        return std::make_shared<UvThreadDispatcher>();
    }

}