/*
 *
 * MongooseSimpleRestServer
 * ledger-core
 *
 * Created by Pierre Pollastri on 02/12/2016.
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
#include "MongooseSimpleRestServer.hpp"
#include "NativeThreadDispatcher.hpp"
#include <sstream>

static void ev_handler(struct mg_connection *c, int ev, void *p) {
    if (ev == MG_EV_HTTP_REQUEST) {
        auto server = (MongooseSimpleRestServer *) (c->user_data);
        server->ev_handler(c, ev, p);
    }
}

void MongooseSimpleRestServer::ev_handler(struct mg_connection *c, int ev, void *p) {
    if (ev == MG_EV_HTTP_REQUEST) {
        struct http_message *hm = (struct http_message *) p;
        auto matcher = _route.set(std::string(hm->uri.p, hm->uri.len));
        std::string method(hm->method.p, hm->method.len);
        for (const auto &endpoint : _endpoints) {
            if (method == endpoint.method && matcher.test(endpoint.path)) {
                RestRequest request(c, hm, matcher);
                auto response = endpoint.handler(request);
                std::stringstream ss;
                ss << "HTTP/1.1 " << response.statusCode << " " << response.statusText << "\r\n";
                ss << "Transfer-Encoding: chunked\r\n\r\n";
                mg_printf(c, "%s", ss.str().c_str());
                mg_printf_http_chunk(c, response.body.c_str());
                mg_send_http_chunk(c, "", 0); /* Send empty chunk, the end of response */
                return ;
            }
        }
        mg_printf(c, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
        mg_printf_http_chunk(c, "{\"error\": \"unknown call\"}");
        mg_send_http_chunk(c, "", 0); /* Send empty chunk, the end of response */
    }
}

void MongooseSimpleRestServer::poll() {
    if (_running) {
        _context->execute(make_runnable([this]() {
            mg_mgr_poll(&_mgr, 100);
            poll();
        }));
    }
}


void MongooseSimpleRestServer::start(short port) {
    if (!_running) {
        _running = true;
        _context->execute(make_runnable([this, port] () {
            mg_mgr_init(&_mgr, NULL);
            _connection = mg_bind(&_mgr, std::to_string(port).c_str(), ::ev_handler);
            _connection->user_data = this;
            mg_set_protocol_http_websocket(_connection);
            poll();
        }));
    }
}

void MongooseSimpleRestServer::stop() {
    if (_running) {
        _running = false;
        _context->execute(make_runnable([this] () {
            _connection->flags = MG_F_CLOSE_IMMEDIATELY;
            mg_mgr_poll(&_mgr, 0);
        }));
    }
}

void
MongooseSimpleRestServer::endpoint(const std::string &method, const std::string &path, const RestRequestHandler &handler) {
    _endpoints.push_back(Endpoint(method, path, handler));
}

void MongooseSimpleRestServer::GET(const std::string &path, const RestRequestHandler &handler) {
    endpoint("GET", path, handler);
}

void MongooseSimpleRestServer::PUT(const std::string &path, const RestRequestHandler &handler) {
    endpoint("PUT", path, handler);
}

void MongooseSimpleRestServer::POST(const std::string &path, const RestRequestHandler &handler) {
    endpoint("POST", path, handler);
}

void MongooseSimpleRestServer::DELETE(const std::string &path, const RestRequestHandler &handler) {
    endpoint("DELETE", path, handler);
}

MongooseSimpleRestServer::MongooseSimpleRestServer(const std::shared_ptr<ledger::core::api::ExecutionContext>& context) {
    _running = false;
    mg_mgr_init(&_mgr, NULL);
    _context = context;
}

MongooseSimpleRestServer::~MongooseSimpleRestServer() {

}
