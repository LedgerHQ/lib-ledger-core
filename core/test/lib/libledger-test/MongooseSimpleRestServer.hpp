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
#ifndef LEDGER_CORE_MONGOOSESIMPLERESTSERVER_HPP
#define LEDGER_CORE_MONGOOSESIMPLERESTSERVER_HPP

#include <string>
#include <functional>
#include "mongoose.h"
#include <ledger/core/api/ExecutionContext.hpp>
#include <unordered_map>
#include <regex>
#include "route.h"

struct RestRequest {
    struct mg_connection *connection;
    struct http_message *message;
    route::Match match;

    std::string getBody() const {
        return std::string(message->message.p, message->message.len);
    };

    RestRequest( struct mg_connection *c,  struct http_message *m, route::Match ma) : connection(c), message(m), match(ma)  {};
};

struct RestResponse {
    const int statusCode;
    const std::string statusText;
    const std::string body;

    RestResponse(int s, const std::string &t, const std::string &b) : statusCode(s), statusText(t), body(b) {};
};

typedef std::function<RestResponse (const RestRequest&)> RestRequestHandler;



class MongooseSimpleRestServer {
public:
    MongooseSimpleRestServer(const std::shared_ptr<ledger::core::api::ExecutionContext>& context);
    void start(short port);
    void stop();
    void endpoint(const std::string& method, const std::string &path, const RestRequestHandler &handler);
    void GET(const std::string &path, const RestRequestHandler &handler);
    void PUT(const std::string &path, const RestRequestHandler &handler);
    void POST(const std::string &path, const RestRequestHandler &handler);
    void DELETE(const std::string &path, const RestRequestHandler &handler);
    ~MongooseSimpleRestServer();

    void ev_handler(struct mg_connection *c, int ev, void *p);

private:
    void poll();

    struct Endpoint {
        const std::string method;
        const std::string path;
        const RestRequestHandler handler;

        Endpoint(const std::string& m, const std::string& p, const RestRequestHandler& h)
                : method(m), path(p), handler(h)
        {};
    };

private:
    struct mg_mgr _mgr;
    bool _running;
    std::shared_ptr<ledger::core::api::ExecutionContext> _context;
    std::vector<Endpoint> _endpoints;
    struct mg_connection *_connection;
    route::Route _route;
};

#endif //LEDGER_CORE_MONGOOSESIMPLERESTSERVER_HPP
