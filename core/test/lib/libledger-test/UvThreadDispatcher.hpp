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

#include <ledger/core/api/ThreadDispatcher.hpp>
#include <ledger/core/api/ExecutionContext.hpp>
#include <ledger/core/async/Future.hpp>
#include <ledger/core/api/Runnable.hpp>
#include <ledger/core/api/Lock.hpp>
#include <condition_variable>

namespace uv {
    std::shared_ptr<ledger::core::api::ThreadDispatcher> createDispatcher();

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
