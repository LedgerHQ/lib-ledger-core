/*
 *
 * wait
 * ledger-core
 *
 * Created by Pierre Pollastri on 24/07/2017.
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
#ifndef LEDGER_CORE_WAIT_H
#define LEDGER_CORE_WAIT_H

#include "Future.hpp"

#include <condition_variable>
#include <utils/ImmediateExecutionContext.hpp>

namespace ledger {
    namespace core {
        namespace async {

            template <typename T>
            T wait(Future<T> future) {
                std::mutex mutex;
                std::unique_lock<std::mutex> lock(mutex);
                std::condition_variable barrier;
                Try<T> res;
                future.onComplete(ImmediateExecutionContext::INSTANCE, [&](const Try<T> &result) {
                    res = result;
                    barrier.notify_all();
                });
                barrier.wait(lock);
                if (res.isSuccess()) {
                    return res.getValue();
                } else {
                    throw res.getFailure();
                }
            }

        } // namespace async
    }     // namespace core
} // namespace ledger

#endif // LEDGER_CORE_WAIT_H
