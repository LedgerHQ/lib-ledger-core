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

static std::string BIG_TEXT =
        "Hi guys, I own a Nano S and I am a big fan, however there is a big slice of information that is missing from your website, that is how reliable the Nano S is, how much testing it's been through (and it goes through any time a new software release is distributed) etc. \n"
        "\n"
        "Think about it, before I put a few thousands Euros in a wallet that depends on the Nano, I want to be sure it won't die on me, say, three months from now, or if someone spills water on it. Think for example how Corsair describes the \"Survivor\" line of USB keys here http://www.corsair.com/en-gb/usb-drives/flash-survivor . Can you offer anything like this? \n"
        "\n"
        "\n"
        "If not, you should recommend your users not to use the Nano S for nothing more than petty cash.\n"
        "\n"
        "What do you reckon?\n"
        "\n"
        "\n"
        "G. ";

TEST(HttpClient, GET) {
    auto dispatcher = std::make_shared<NativeThreadDispatcher>();
    auto client = std::make_shared<MongooseHttpClient>(dispatcher->getSerialExecutionContext("client"));
    auto worker = dispatcher->getSerialExecutionContext("worker");
    ledger::core::HttpClient http("http://localhost:8000", client, worker);
    {
        auto server = std::make_shared<MongooseSimpleRestServer>(dispatcher->getSerialExecutionContext("server"));

        server->GET("/say/hello/:to/please", [dispatcher](const RestRequest &request) -> RestResponse {
            std::string result = "Hello ";
            result += request.match.get("to");
            return {200, "OK", result};
        });

        server->start(8000);

        worker->execute(make_runnable([&http, dispatcher, &server] () {
            http.GET("/say/hello/toto/please")([dispatcher,  &server] (const ledger::core::api::HttpResponse& response) {
                auto res = std::string((char *)response.body.data(), response.body.size());
                EXPECT_EQ(200, response.statusCode);
                EXPECT_EQ("Hello toto", res);
                std::cout << res << std::endl;
                server->stop();
                dispatcher->getMainExecutionContext()->delay(make_runnable([dispatcher]() {
                    dispatcher->stop();
                }), 0);
            });
        }));
        WAIT_AND_TIMEOUT(dispatcher, 10000)
    }
}

TEST(HttpClient, POST) {
    auto dispatcher = std::make_shared<NativeThreadDispatcher>();
    auto client = std::make_shared<MongooseHttpClient>(dispatcher->getSerialExecutionContext("client"));
    auto worker = dispatcher->getSerialExecutionContext("worker");
    ledger::core::HttpClient http("http://localhost:8000", client, worker);
    {
        auto server = std::make_shared<MongooseSimpleRestServer>(dispatcher->getSerialExecutionContext("server"));

        server->POST("/next/century/postal/service", [dispatcher](const RestRequest &request) -> RestResponse {
            std::string result(request.message->body.p, request.message->body.len);
            return {200, "OK", result};
        });

        server->start(8000);


        worker->execute(make_runnable([&http, dispatcher, &server] () {
            std::vector<uint8_t> body((uint8_t *)BIG_TEXT.c_str(), (uint8_t *)(BIG_TEXT.c_str() + BIG_TEXT.size()));
            http.POST("/next/century/postal/service", body)([dispatcher,  &server] (const ledger::core::api::HttpResponse& response) {
                auto res = std::string((char *)response.body.data(), response.body.size());
                EXPECT_EQ(200, response.statusCode);
                EXPECT_EQ(BIG_TEXT, res);
                server->stop();
                dispatcher->getMainExecutionContext()->delay(make_runnable([dispatcher]() {
                    dispatcher->stop();
                }), 0);
            });
        }));

        WAIT_AND_TIMEOUT(dispatcher, 10000)
    }
}