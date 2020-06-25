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

enum class EventType {
    RUN,
    CLOSE
};

struct Event {
    EventType eventType;
    std::shared_ptr<api::Runnable> runnable;
};

static void context_callback(uv_async_t* handle);

class EventLoop {
public:
    EventLoop() : _loop(new uv_loop_t), _async(new uv_async_t), _running(true) {
        uv_loop_init(_loop);
        uv_async_init(_loop, _async, context_callback);
        _async->data = static_cast<void *>(this);
    }

    EventLoop(const EventLoop&) = delete;
    EventLoop(EventLoop&&) = delete;
    EventLoop & operator=(const EventLoop&) = delete;

    void queue(const std::shared_ptr<api::Runnable>& runnable) {
        std::unique_lock<std::mutex> lock(_mutex);
        if (_running) {
            Event event;
            event.eventType = EventType::RUN;
            event.runnable = runnable;
            _queue.emplace(std::move(event));
            uv_async_send(_async);
        }
    }

    void queue(const std::shared_ptr<api::Runnable>& runnable, int64_t millis) {
        throw std::runtime_error("Queuing with delay is not implemented.");
    }

    bool run() {
        uv_run(_loop, UV_RUN_DEFAULT);
        return uv_loop_alive(_loop) != 0;
    }

    void tick() {
        Option<Event> event;
        while ((event = dequeue()).hasValue()) {
            switch (event.getValue().eventType) {
                case EventType::RUN: {
                    auto result = Try<Unit>::from([&]() {
                        event.getValue().runnable->run();
                        return unit;
                    });
                    if (result.isFailure()) {
                        std::cerr << "An error happened during asynchronous execution: "
                                  << result.getFailure().getMessage() << std::endl;
                    }
                    break;
                }
                case EventType::CLOSE: {
                    std::unique_lock<std::mutex> lock(_mutex);
                    _running = false;
                    uv_stop(_loop);
                    uv_loop_close(_loop);
                    uv_close(reinterpret_cast<uv_handle_t*>(_async), NULL);
                    break;
                }
            }
        }
    }

    void close() {
        std::unique_lock<std::mutex> lock(_mutex);
        Event event;
        event.eventType = EventType::CLOSE;
        _queue.emplace(event);
        uv_async_send(_async);
    }

    Option<Event> dequeue() {
        std::unique_lock<std::mutex> lock(_mutex);
        if (!_queue.empty() && _running) {
            auto event = _queue.front();
            _queue.pop();
            return event;
        } else {
            return Option<Event>::NONE;
        }
    }

    ~EventLoop() {
        delete _loop;
        delete _async;
    }

private:
    uv_loop_t* _loop;
    uv_async_t* _async;
    uv_timer_t* timer;
    std::mutex _mutex;
    bool _running;
    std::queue<Event> _queue;
};

static void context_callback(uv_async_t* handle) {
    static_cast<EventLoop *>(handle->data)->tick();
}

class SequentialExecutionContext : public api::ExecutionContext {
public:

    SequentialExecutionContext() {
        _thread = std::thread([this] () {
            run();
        });
    }

    void run() {
        while (_loop.run());
    }

    void stop() {
        _loop.close();
        _thread.join();
    }

    void execute(const std::shared_ptr<api::Runnable> &runnable) override {
        _loop.queue(runnable);
    }

    void delay(const std::shared_ptr<api::Runnable> &runnable, int64_t millis) override {
        _loop.queue(runnable, millis);
    }

    ~SequentialExecutionContext() override {

    }

private:
    std::thread _thread;
    EventLoop _loop;
};

class ThreadDispatcher : public api::ThreadDispatcher {
public:
    std::shared_ptr<api::ExecutionContext> getSerialExecutionContext(const std::string &name) override {
        std::unique_lock<std::mutex> lock(_mutex);
        auto it = _serialContexts.find(name);
        if (it == _serialContexts.end()) {
            auto context = std::make_shared<SequentialExecutionContext>();
            _serialContexts[name] = context;
            return context;
        }
        return it->second;
    }

    std::shared_ptr<api::ExecutionContext> getThreadPoolExecutionContext(const std::string &name) override {
        // TODO Implement thread pools
        return getSerialExecutionContext(name);
    }

    std::shared_ptr<api::ExecutionContext> getMainExecutionContext() override {
        return getSerialExecutionContext("__uv__main__");
    }

    std::shared_ptr<api::Lock> newLock() override {
        return nullptr;
    }

    ~ThreadDispatcher() {
       for (auto& context : _serialContexts) {
           context.second->stop();
       }
    }

private:
    std::mutex _mutex;
    std::unordered_map<std::string, std::shared_ptr<SequentialExecutionContext>> _serialContexts;
};

namespace uv {

    std::shared_ptr<ledger::core::api::ThreadDispatcher> createDispatcher() {
        return std::make_shared<ThreadDispatcher>();
    }

}