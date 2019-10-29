/*
 *
 * MongooseHttpClient
 * ledger-core
 *
 * Created by Pierre Pollastri on 01/12/2016.
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

#pragma once

#include <memory>

#include <core/api/HttpClient.hpp>
#include <core/api/ExecutionContext.hpp>

#include "mongoose.h"

class MongooseHttpClient : public ledger::core::api::HttpClient, public std::enable_shared_from_this<MongooseHttpClient> {

public:
    MongooseHttpClient(const std::shared_ptr<ledger::core::api::ExecutionContext>& context);
    virtual void execute(const std::shared_ptr<ledger::core::api::HttpRequest> &request) override;
    void ev_handler(struct mg_connection *c, int ev, struct http_message *hm, const std::shared_ptr<ledger::core::api::HttpRequest>& request);
    ~MongooseHttpClient();

private:
    void start();
    void stop();
    void poll();

private:
    std::shared_ptr<ledger::core::api::ExecutionContext> _context;
    struct mg_mgr _mgr;
    bool _running;
};
