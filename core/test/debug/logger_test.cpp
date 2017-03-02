/*
 *
 * json_preferences_test
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

#include <gtest/gtest.h>
#include <EventLooper.hpp>
#include <EventThread.hpp>
#include <NativeThreadDispatcher.hpp>
#include <ledger/core/preferences/AtomicPreferencesBackend.hpp>
#include <NativePathResolver.hpp>
#include <fstream>
#include <ledger/core/debug/logger.hpp>
#include <CoutLogPrinter.hpp>
#include <iostream>
#include <ledger/core/utils/optional.hpp>
#include <string>
#include <fstream>
#include <streambuf>

TEST(LoggerTest, LogAndOverflow) {
    auto dispatcher = std::make_shared<NativeThreadDispatcher>();
    auto logPrinter = std::make_shared<CoutLogPrinter>(dispatcher->getMainExecutionContext());
    auto resolver = std::make_shared<NativePathResolver>();

    std::shared_ptr<spdlog::logger> logger = ledger::core::logger::create("test_logs",
                                               std::experimental::optional<std::string>(),
                                               dispatcher->getSerialExecutionContext("logger"),
                                               resolver,
                                               logPrinter,
                                                                          (std::string("2017-03-02T10:07:06Z+01:00 D: This is a log \n").size() + 3) * 199
    );
    dispatcher->getMainExecutionContext()->execute(make_runnable([=] () {
        for (auto i = 0; i < 200; i++) {
            logger->debug("This is a log {0:03d}", i);
        }

        dispatcher->getSerialExecutionContext("logger")->delay(make_runnable([=] () {
            dispatcher->stop();
        }), 0);
    }));
    dispatcher->waitUntilStopped();
    std::ifstream t(resolver->resolveLogFilePath("test_logs.log"));
    std::string str((std::istreambuf_iterator<char>(t)),
                    std::istreambuf_iterator<char>());
    std::cout << "File size: " << str.size() << std::endl;
    std::cout << str << std::endl;
    EXPECT_TRUE(str.find("This is a log 199") != std::string::npos);
    EXPECT_FALSE(str.find("This is a log 0") != std::string::npos);
    resolver->clean();
}

TEST(LoggerTest, LogNoOverflow) {
    auto dispatcher = std::make_shared<NativeThreadDispatcher>();
    auto logPrinter = std::make_shared<CoutLogPrinter>(dispatcher->getMainExecutionContext());
    auto resolver = std::make_shared<NativePathResolver>();

    std::shared_ptr<spdlog::logger> logger = ledger::core::logger::create("test_logs_1",
                                                                          std::experimental::optional<std::string>(),
                                                                          dispatcher->getSerialExecutionContext("logger"),
                                                                          resolver,
                                                                          logPrinter,
                                                                          (std::string("2017-03-02T10:07:06Z+01:00 D: This is a log \n").size() + 3) * 200
    );
    dispatcher->getMainExecutionContext()->execute(make_runnable([=] () {
        for (auto i = 0; i < 200; i++) {
            logger->debug("This is a log {0:03d}", i);
        }

        dispatcher->getSerialExecutionContext("logger")->delay(make_runnable([=] () {
            dispatcher->stop();
        }), 0);
    }));
    dispatcher->waitUntilStopped();
    std::ifstream t(resolver->resolveLogFilePath("test_logs_1.log"));
    std::string str((std::istreambuf_iterator<char>(t)),
                    std::istreambuf_iterator<char>());
    std::cout << "File size: " << str.size() << std::endl;
    std::cout << str << std::endl;
    EXPECT_TRUE(str.find("This is a log 199") != std::string::npos);
    EXPECT_TRUE(str.find("This is a log 0") != std::string::npos);
    resolver->clean();
}