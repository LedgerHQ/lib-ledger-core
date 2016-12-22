/*
 *
 * PoolTestCaseBootstraper
 * ledger-core
 *
 * Created by Pierre Pollastri on 21/12/2016.
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
#ifndef LEDGER_CORE_POOLTESTCASEBOOTSTRAPER_HPP
#define LEDGER_CORE_POOLTESTCASEBOOTSTRAPER_HPP

#include "NativeThreadDispatcher.hpp"
#include "NativePathResolver.hpp"
#include "CoutLogPrinter.hpp"
#include "MongooseHttpClient.hpp"
#include <ledger/core/api/WalletPool.hpp>
#include <memory>
#include <ledger/core/utils/Exception.hpp>

class PoolTestCaseBootstraper {
public:
    PoolTestCaseBootstraper(const std::string &poolName);
    void setup(std::function<void (std::shared_ptr<ledger::core::api::WalletPool>, std::experimental::optional<ledger::core::api::Error>)> callback);
    void tearDown();

public:
    std::shared_ptr<NativeThreadDispatcher> dispatcher;
    std::shared_ptr<NativePathResolver> resolver;
    std::shared_ptr<CoutLogPrinter> printer;
    std::shared_ptr<MongooseHttpClient> client;

private:
    std::string _poolName;
};


#endif //LEDGER_CORE_POOLTESTCASEBOOTSTRAPER_HPP
