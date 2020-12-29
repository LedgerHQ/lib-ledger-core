/*
 *
 * Concurrency
 * ledger-core
 *
 * Created by Huiqi ZHENG on 28/12/2020.
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
#ifndef LEDGER_CORE_CONCURRENCY_H
#define LEDGER_CORE_CONCURRENCY_H

#include <thread>
#include <mutex>

namespace ledger {
    namespace core {
        class Concurrency {
        public:
            template <typename InputIter, typename UnaryFunction>
            static void parallel_for_each(InputIter first, InputIter last, UnaryFunction f)
            {
                auto range = std::distance(first, last);
                size_t approxNumThreads = std::thread::hardware_concurrency();
                // number of elements for one thread
                size_t jobsForThread = range / approxNumThreads;
                // number of elements for one thread + remainder
                size_t jobsForMainThread = range % approxNumThreads + jobsForThread;
                // minus main thread
                std::vector<std::thread> threads(approxNumThreads - 1);
                InputIter block_start = first;
                InputIter block_end = first + jobsForThread;
                for(size_t i = 0; i < approxNumThreads - 1; ++i)
                {
                    threads[i] = std::thread(std::for_each<InputIter, UnaryFunction>,
                                             block_start, block_end, f);
                    block_start = block_end;
                    block_end += jobsForThread;
                }
                std::for_each(block_start, block_start + jobsForMainThread, f);
                for(auto& t : threads)
                {
                    if(t.joinable())
                    {
                        t.join();
                    }
                }
            }
        };
    }
}


#endif LEDGER_CORE_CONCURRENCY_H
