/*
 *
 * ExplorerFixture
 *
 * ledger-core
 *
 * Created by Alexis Le Provost on 04/02/2020.
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

#pragma once

#include <integration/BaseFixture.hpp>

template <typename CurrencyExplorer, typename NetworkParameters>
class ExplorerFixture : public BaseFixture {
public:
    void SetUp() override {
        BaseFixture::SetUp();

        auto worker = dispatcher->getSerialExecutionContext("worker");
        auto client = std::make_shared<HttpClient>(explorerEndpoint, http, worker);
        explorer = std::make_shared<CurrencyExplorer>(worker, client, params, api::DynamicObject::newInstance());
        logger = ledger::core::logger::create("test_logs",
                                              dispatcher->getSerialExecutionContext("logger"),
                                              resolver,
                                              printer,
                                              2000000000
        );
    }

    void TearDown() override {
        logger.reset();
        explorer.reset();
    }

    NetworkParameters params;
    std::string explorerEndpoint;
    std::shared_ptr<spdlog::logger> logger;
    std::shared_ptr<CurrencyExplorer> explorer;
};