/*
 *
 * http_client_tests
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
#include <gtest/gtest.h>
#include <EventLooper.hpp>
#include <EventThread.hpp>
#include <NativeThreadDispatcher.hpp>
#include <NativePathResolver.hpp>
#include <fstream>
#include <mongoose.h>
#include <MongooseHttpClient.hpp>
#include <MongooseSimpleRestServer.hpp>
#include <ledger/core/net/HttpClient.hpp>

TEST(HttpClient, GET) {

    auto dispatcher = std::make_shared<NativeThreadDispatcher>();
    auto client = std::make_shared<MongooseHttpClient>(dispatcher->getSerialExecutionContext("client"));
    auto worker = dispatcher->getSerialExecutionContext("worker");
    ledger::core::HttpClient http("http://localhost:8000", client, worker);
    {
        MongooseSimpleRestServer server(dispatcher->getSerialExecutionContext("server"));

        server.GET("/say/hello/:to/please", [dispatcher](const RestRequest &request) -> RestResponse {
            std::string result = "Hello ";
            result += request.match.get("to");
            return {200, "OK", result};
        });

        server.start(8000);

        worker->execute(make_runnable([&http, dispatcher] () {
            http.GET("/say/hello/toto/please")([dispatcher] (const ledger::core::api::HttpResponse& response) {
                auto res = std::string((char *)response.body.data(), response.body.size());
                EXPECT_EQ(200, response.statusCode);
                EXPECT_EQ("Hello toto", res);
                dispatcher->getMainExecutionContext()->delay(make_runnable([dispatcher]() {
                    dispatcher->stop();
                }), 0);
            });
        }));
        WAIT_AND_TIMEOUT(dispatcher, 10000)
        server.stop();
    }
}