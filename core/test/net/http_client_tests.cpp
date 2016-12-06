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
#include <ledger/core/preferences/AtomicPreferencesBackend.hpp>
#include <NativePathResolver.hpp>
#include <fstream>
#include <mongoose.h>
#include <MongooseHttpClient.hpp>
#include <MongooseSimpleRestServer.hpp>

TEST(HttpClient, GET) {

    auto dispatcher = std::make_shared<NativeThreadDispatcher>();

    {
        MongooseSimpleRestServer server(dispatcher->getSerialExecutionContext("server"));

        server.GET("/say/hello/:to/please", [dispatcher](const RestRequest &request) -> RestResponse {
            std::string result = "Hello ";
            result += request.match.get("to");
            dispatcher->getMainExecutionContext()->delay(make_runnable([dispatcher]() {
                dispatcher->stop();
            }), 0);
            return {200, "OK", result};
        });

        server.start(8000);
        dispatcher->waitUntilStopped();

        server.stop();
    }
    {
        MongooseSimpleRestServer server(dispatcher->getSerialExecutionContext("server"));
        server.GET("/say/hello/:to/please", [dispatcher](const RestRequest &request) -> RestResponse {
            std::string result = "Not saying hello ";
            result += request.match.get("to");
            dispatcher->getMainExecutionContext()->delay(make_runnable([dispatcher]() {
                //dispatcher->stop();
            }), 0);
            return {200, "OK", result};
        });

        server.start(8000);
        dispatcher->waitUntilStopped();
    }
}