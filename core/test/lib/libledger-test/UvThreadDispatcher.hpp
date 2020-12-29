/*
 *
 * UvThreadDispatcher.hpp
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

#ifndef LEDGER_CORE_UVTHREADDISPATCHER_HPP
#define LEDGER_CORE_UVTHREADDISPATCHER_HPP

#include <uv.h>
#include <ledger/core/api/ThreadDispatcher.hpp>
#include <ledger/core/api/ExecutionContext.hpp>
#include <ledger/core/async/Future.hpp>
#include <ledger/core/api/Runnable.hpp>
#include <ledger/core/api/Lock.hpp>
#include <condition_variable>

#include <thread>
#include <ledger/core/utils/Option.hpp>
#include <ledger/core/utils/Try.hpp>
#include <unordered_map>

namespace uv {
    std::shared_ptr<ledger::core::api::ThreadDispatcher> createDispatcher();

    enum class EventType {
        RUN,
        CLOSE
    };

    struct Event {
        EventType eventType;
        std::shared_ptr<ledger::core::api::Runnable> runnable;
        unsigned int delay;
    };

    static void context_callback(uv_async_t* handle);
    static void timer_callback(uv_timer_t* handle);

    class EventLoop {
    public:
        EventLoop();

        EventLoop(const EventLoop&) = delete;
        EventLoop(EventLoop&&) = delete;
        EventLoop & operator=(const EventLoop&) = delete;

        void queue(const std::shared_ptr<ledger::core::api::Runnable>& runnable);

        void queue(const std::shared_ptr<ledger::core::api::Runnable>& runnable, int64_t millis);

        bool run();

        void tick();

        void waitUntilStopped();

        void stop();

        ledger::core::Option<Event> dequeue();

        ~EventLoop();

    private:
        uv_loop_t* _loop;
        uv_async_t* _async;
        uv_timer_t* _timer;
        std::mutex _mutex;
        std::condition_variable condition;
        bool _running;
        std::queue<Event> _queue;
    };

    class SequentialExecutionContext : public ledger::core::api::ExecutionContext {
    public:

        SequentialExecutionContext();

        void run();

        void stop();

        void execute(const std::shared_ptr<ledger::core::api::Runnable> &runnable) override;

        void delay(const std::shared_ptr<ledger::core::api::Runnable> &runnable, int64_t millis) override;

        ~SequentialExecutionContext() override;

        void waitUntilStopped();

    private:
        std::thread _thread;
        EventLoop _loop;
    };

    class UvThreadDispatcher : public ledger::core::api::ThreadDispatcher {
    public:

        UvThreadDispatcher(unsigned int poolSize = 4);

        std::shared_ptr<ledger::core::api::ExecutionContext> getSerialExecutionContext(const std::string &name) override;

        std::shared_ptr<ledger::core::api::ExecutionContext> getThreadPoolExecutionContext(const std::string &name) override;

        std::shared_ptr<ledger::core::api::ExecutionContext> getMainExecutionContext() override;

        std::shared_ptr<ledger::core::api::Lock> newLock() override;

        ~UvThreadDispatcher();

        void waitUntilStopped();

        void stop();

    private:
        unsigned int _poolSize;
        std::mutex _mutex;
        std::vector<std::shared_ptr<SequentialExecutionContext>> _pool;
        std::unordered_map<std::string, std::shared_ptr<SequentialExecutionContext>> _contextsByName;
    };


    template <typename T>
    static T wait(ledger::core::Future<T> future) {
        std::mutex mutex;
        std::condition_variable condition;
        auto dispatcher = createDispatcher();
        ledger::core::Try<T> result;
        future.onComplete(dispatcher->getMainExecutionContext(), [&] (auto res) mutable {
            std::unique_lock<std::mutex> lock(mutex);
            result = res;
            condition.notify_all();
        });
        {
            std::unique_lock<std::mutex> lock(mutex);
            condition.wait(lock, [&] () {
                return result.isComplete();
            });
        }
        if (result.isFailure()) {
            throw result.getFailure();
        }
        return result.getValue();
    }
}


#endif //LEDGER_CORE_UVTHREADDISPATCHER_HPP
