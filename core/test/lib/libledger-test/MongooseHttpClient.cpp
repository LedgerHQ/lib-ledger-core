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
#include "MongooseHttpClient.hpp"
#include "NativeThreadDispatcher.hpp"
#include <ledger/core/api/HttpRequest.hpp>
#include "mongoose.h"
#include <unordered_map>

static std::shared_ptr<ledger::core::api::ExecutionContext> sContext;
static bool sExitFlag = false;
static std::unordered_map<struct mg_connection *, std::shared_ptr<ledger::core::api::HttpRequest>> sRequests;
static struct mg_mgr sMgr;

static void ev_handler(struct mg_connection *c, int ev, void *p) {
    if (ev == MG_EV_HTTP_REPLY) {

    }
}

void MongooseHttpClient::execute(const std::shared_ptr<ledger::core::api::HttpRequest> &request) {
    if (sContext) {
        sContext->execute(make_runnable([request] () {
            struct mg_mgr mgr;
            struct mg_connection *nc;

            mg_mgr_init(&mgr, NULL);
            nc = mg_connect_http(&mgr, ev_handler, request->getUrl().c_str(), NULL, NULL);
            sRequests[nc] = request;
            mg_set_protocol_http_websocket(nc);
        }));
    }
}

static void schedulePoll() {
    if (sContext) {
        sContext->delay(make_runnable([]() {
            mg_mgr_poll(&sMgr, 0);
            if (!sExitFlag)
                schedulePoll();
        }), 1000);
    }
}

void MongooseHttpClient::startService(const std::shared_ptr<ledger::core::api::ExecutionContext> &context) {
    if (!sContext) {
        sContext = context;
        sExitFlag = false;
        mg_mgr_init(&sMgr, NULL);
        schedulePoll();
    }
}

void MongooseHttpClient::stopService() {
    if (sContext) {
        sContext = nullptr;
        sExitFlag = true;
        mg_mgr_free(&sMgr);
    }
}

