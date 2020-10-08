/*
 *
 * threading_tests
 * ledger-core
 *
 * Created by Pierre Pollastri on 20/04/2017.
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


#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#else
    #include <pthread.h>
#endif
#include <gtest/gtest.h>
#include <UvThreadDispatcher.hpp>
#include <time.h>

using namespace ledger::core;
//using namespace ledger::qt;

unsigned long getCurrentThreadId(){
#ifdef _WIN32
    return (unsigned long)GetCurrentThreadId();
#else
    return (unsigned long)pthread_self();
#endif
}

TEST(Threading, DoSomethingOnSerialQueue) {
    auto dispatcher = std::make_shared<uv::UvThreadDispatcher>();
    int var = 0;

    auto mainThread = getCurrentThreadId();
    dispatcher->getThreadPoolExecutionContext("worker")->execute(make_runnable([&] () {
        EXPECT_NE(mainThread, getCurrentThreadId());
        var = 1;
        dispatcher->stop();
    }));

    dispatcher->waitUntilStopped();
    EXPECT_EQ(var, 1);
    EXPECT_EQ(dispatcher->getThreadPoolExecutionContext("worker"), dispatcher->getThreadPoolExecutionContext("worker"));   
}

TEST(Threading, DoSomethingOnSerialQueueWithDelay) {
    auto dispatcher = std::make_shared<uv::UvThreadDispatcher>();

    auto before = time(0);
    auto after = before;

    auto mainThread = getCurrentThreadId();

    dispatcher->getSerialExecutionContext("worker")->delay(make_runnable([&] () {
        EXPECT_NE(mainThread, getCurrentThreadId());
        after = time(0);
        dispatcher->stop();
    }), 1000);

    dispatcher->waitUntilStopped();

    EXPECT_TRUE(after >= (before + 1000));
    EXPECT_EQ(dispatcher->getSerialExecutionContext("worker"), dispatcher->getSerialExecutionContext("worker"));
}

TEST(Threading, DoSomethingOnThreadPoolSerialQueue) {
    auto dispatcher = std::make_shared<uv::UvThreadDispatcher>(2);
    auto cpt = 0;
    std::mutex mutex;
    std::condition_variable condition;

    auto mainThread = getCurrentThreadId();
    unsigned long threadId1, threadId2, threadId3;

    auto getThreadId = [&] (auto context, unsigned long & threadId) {
        context->execute(make_runnable([&] () {
        std::unique_lock<std::mutex> lock(mutex);
        threadId = getCurrentThreadId();
        ++ cpt;
        condition.notify_all();
    }));
    };

    getThreadId(dispatcher->getThreadPoolExecutionContext("worker-1"), threadId1);
    getThreadId(dispatcher->getThreadPoolExecutionContext("worker-2"), threadId2);
    getThreadId(dispatcher->getThreadPoolExecutionContext("worker-3"), threadId3);

    std::unique_lock<std::mutex> lock(mutex);
    condition.wait(lock, [&] () {
        return (cpt == 3);
    });
    
    EXPECT_NE(mainThread, threadId1);
    EXPECT_NE(mainThread, threadId2);
    EXPECT_NE(mainThread, threadId3);
    EXPECT_NE(threadId1, threadId2);
    EXPECT_EQ(threadId1, threadId3);
}
