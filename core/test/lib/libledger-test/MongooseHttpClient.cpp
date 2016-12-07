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
#include "../../../src/api/HttpMethod.hpp"
#include <ledger/core/api/HttpRequest.hpp>
#include <ledger/core/api/HttpResponse.hpp>
#include <sstream>
#include <cstring>

static void ev_handler(struct mg_connection *c, int ev, void *p) {
    if (ev == MG_EV_CONNECT) {
        int status = *(int *)p;
        if (p != 0) {
            auto pair = (std::pair<std::shared_ptr<ledger::core::api::HttpRequest>, std::shared_ptr<MongooseHttpClient>> *)c->user_data;
            ledger::core::api::HttpResponse response(
                    0,
                   "Unable to connect",
                    std::unordered_map<std::string, std::string>(),
                    std::vector<uint8_t >()
            );
            //pair->first->complete(response);
            //delete pair;
        }
    } else if (ev == MG_EV_HTTP_REPLY) {
        c->flags |= MG_F_CLOSE_IMMEDIATELY;
        struct http_message *hm = (struct http_message *) p;
        auto pair = (std::pair<std::shared_ptr<ledger::core::api::HttpRequest>, std::shared_ptr<MongooseHttpClient>> *)c->user_data;
        if (pair->second)
            pair->second->ev_handler(c, ev, hm, pair->first);
        delete pair;
    }
}

void MongooseHttpClient::ev_handler(struct mg_connection *c, int ev, struct http_message *hm, const std::shared_ptr<ledger::core::api::HttpRequest>& request) {
    ledger::core::api::HttpResponse response(
            hm->resp_code,
            std::string(hm->resp_status_msg.p, hm->resp_status_msg.len),
            std::unordered_map<std::string, std::string>(),
            std::vector<uint8_t >((uint8_t *)hm->body.p, (uint8_t *)(hm->body.p + hm->body.len))
    );
    request->complete(response);
}

void MongooseHttpClient::execute(const std::shared_ptr<ledger::core::api::HttpRequest> &request) {
    start();
    auto self = shared_from_this();
    _context->execute(make_runnable([request, this, self] () {
        struct mg_connection *nc;
        mg_mgr_init(&_mgr, NULL);
        // Compute method
        std::string method;
        switch (request->getMethod()) {
            case ledger::core::api::HttpMethod::GET:
                method = "GET";
                break;
            case ledger::core::api::HttpMethod::POST:
                method = "POST";
                break;
            case ledger::core::api::HttpMethod::PUT:
                method = "PUT";
                break;
            case ledger::core::api::HttpMethod::DEL:
                method = "DELETE";
                break;
        }

        // Compute headers string content
        std::stringstream headers;
        for (auto& header : request->getHeaders()) {
            headers << header.first << ": " << header.second << "\r\n";
        }

        // Compute body string content
        std::stringstream body;
        const char *c_body = NULL;
        if (request->getBody().size() > 0) {
            for (auto byte : request->getBody()) {
                body << (char )byte;
            }
            c_body = ::strdup(body.str().c_str());
        }

        nc = mg_connect_http(&_mgr, ::ev_handler, method.c_str(), request->getUrl().c_str(), headers.str().c_str(), c_body); // Pass headers and body data
        nc->user_data = new std::pair<std::shared_ptr<ledger::core::api::HttpRequest>, std::shared_ptr<MongooseHttpClient>>(request, self);
        mg_set_protocol_http_websocket(nc);
    }));
}

MongooseHttpClient::MongooseHttpClient(const std::shared_ptr<ledger::core::api::ExecutionContext> &context) {
    _context = context;
    _running = false;
}

MongooseHttpClient::~MongooseHttpClient() {
    stop();
}

void MongooseHttpClient::start() {
    if (!_running) {
        _running = true;
        mg_mgr_init(&_mgr, NULL);
        poll();
    }
}

void MongooseHttpClient::stop() {
    if (_running) {
        _running = false;
        mg_mgr_free(&_mgr);
    }
}

void MongooseHttpClient::poll() {
    if (_running) {
        std::weak_ptr<MongooseHttpClient> self = shared_from_this();
        _context->delay(make_runnable([self]() {
            auto s = self.lock();
            if (s) {
                mg_mgr_poll(&(s->_mgr), 0);
                s->poll();
            }
        }), 1000);
    }
}