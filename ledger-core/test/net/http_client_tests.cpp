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

#include <boost/lexical_cast.hpp>
#include <gtest/gtest.h>
#include <fstream>

#include <core/net/HttpClient.hpp>
#include <core/net/HttpJsonHandler.hpp>

#include <EventLooper.hpp>
#include <EventThread.hpp>
#include <NativeThreadDispatcher.hpp>
#include <NativePathResolver.hpp>
#include <mongoose.h>
#include <MongooseHttpClient.hpp>
#include <MongooseSimpleRestServer.hpp>

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

using namespace ledger::core;

TEST(HttpClient, GET) {
    auto dispatcher = std::make_shared<NativeThreadDispatcher>();
    auto client = std::make_shared<MongooseHttpClient>(dispatcher->getSerialExecutionContext("client"));
    auto worker = dispatcher->getSerialExecutionContext("worker");
    ledger::core::HttpClient http("http://127.0.0.1:8000", client, worker);
    {
        auto server = std::make_shared<MongooseSimpleRestServer>(dispatcher->getSerialExecutionContext("server"));

        server->GET("/say/hello/:to/please", [dispatcher](const RestRequest &request) -> RestResponse {
            std::string result = "Hello ";
            result += request.match.get("to");
            return {200, "OK", result};
        });

        server->start(8000);

        worker->execute(::make_runnable([&http, dispatcher, &server] () {
            http.GET("/say/hello/toto/please")().foreach(dispatcher->getMainExecutionContext(),
                                                            [dispatcher, &server] (const std::shared_ptr<api::HttpUrlConnection> connection) {
                auto body = connection->readBody();
                auto res = std::string((char *)(body.data->data()), body.data->size());
                EXPECT_EQ(200, connection->getStatusCode());
                EXPECT_EQ("Hello toto", res);
                std::cout << res << std::endl;
                server->stop();
                dispatcher->getMainExecutionContext()->delay(::make_runnable([dispatcher]() {
                    dispatcher->stop();
                }), 0);
            });
        }));
        WAIT_AND_TIMEOUT(dispatcher, 10000)
    }
}

TEST(HttpClient, GETJson) {
    auto dispatcher = std::make_shared<NativeThreadDispatcher>();
    auto client = std::make_shared<MongooseHttpClient>(dispatcher->getSerialExecutionContext("client"));
    auto worker = dispatcher->getSerialExecutionContext("worker");
    ledger::core::HttpClient http("http://127.0.0.1:8000", client, worker);
    {
        auto server = std::make_shared<MongooseSimpleRestServer>(dispatcher->getSerialExecutionContext("server"));

        server->GET("/talk/to/me/in/json", [dispatcher](const RestRequest &request) -> RestResponse {
            return {200, "OK", "{\"the_answer\": 42}"};
        });

        server->start(8000);

        worker->execute(::make_runnable([&http, dispatcher, &server] () {
                http
                    .GET("/talk/to/me/in/json")
                    .json()
                    .foreach(dispatcher->getMainExecutionContext(), [&server, dispatcher] (const HttpRequest::JsonResult& result) {
                        auto& json = *std::get<1>(result);
                        EXPECT_TRUE(json.HasMember("the_answer"));
                        EXPECT_TRUE(json["the_answer"].IsInt());
                        EXPECT_EQ(42, json["the_answer"].GetInt());
                        server->stop();
                        dispatcher->getMainExecutionContext()->delay(::make_runnable([dispatcher]() {
                            dispatcher->stop();
                        }), 0);
            });
        }));
        WAIT_AND_TIMEOUT(dispatcher, 10000);
    }
}

TEST(HttpClient, GETJsonError) {
    auto dispatcher = std::make_shared<NativeThreadDispatcher>();
    auto client = std::make_shared<MongooseHttpClient>(dispatcher->getSerialExecutionContext("client"));
    auto worker = dispatcher->getSerialExecutionContext("worker");
    ledger::core::HttpClient http("http://127.0.0.1:8000", client, worker);
    {
        auto server = std::make_shared<MongooseSimpleRestServer>(dispatcher->getSerialExecutionContext("server"));

        server->GET("/knock/knock/neo", [dispatcher](const RestRequest &request) -> RestResponse {
            return {301, "Moved Permanently", "{\"not_here\": true}"};
        });

        server->start(8000);

        worker->execute(::make_runnable([&http, dispatcher, &server] () {
            http
                    .GET("/knock/knock/neo")
                    .json()
                    .onComplete(dispatcher->getMainExecutionContext(), [&server, dispatcher] (const Try<HttpRequest::JsonResult>& result) {
                        EXPECT_TRUE(result.isFailure());
                        auto& json = *std::get<1>(*result.getFailure().getTypeUserData<HttpRequest::JsonResult>().getValue());
                        EXPECT_TRUE(json.HasMember("not_here"));
                        EXPECT_TRUE(json["not_here"].IsBool());
                        EXPECT_EQ(true, json["not_here"].GetBool());
                        server->stop();
                        dispatcher->getMainExecutionContext()->delay(::make_runnable([dispatcher]() {
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
    ledger::core::HttpClient http("http://127.0.0.1:8000", client, worker);
    {
        auto server = std::make_shared<MongooseSimpleRestServer>(dispatcher->getSerialExecutionContext("server"));

        server->POST("/next/century/postal/service", [dispatcher](const RestRequest &request) -> RestResponse {
            std::string result(request.message->body.p, request.message->body.len);
            return {200, "OK", result};
        });

        server->start(8000);


        worker->execute(::make_runnable([&http, dispatcher, &server] () {
            std::vector<uint8_t> body((uint8_t *)BIG_TEXT.c_str(), (uint8_t *)(BIG_TEXT.c_str() + BIG_TEXT.size()));
            http.POST("/next/century/postal/service", body)().foreach(dispatcher->getMainExecutionContext(),
                                                            [dispatcher, &server] (const std::shared_ptr<api::HttpUrlConnection> connection) {
                auto body = connection->readBody();
                auto res = std::string((char *)(body.data->data()), body.data->size());
                EXPECT_EQ(200, connection->getStatusCode());
                EXPECT_EQ(BIG_TEXT, res);
                server->stop();
                dispatcher->getMainExecutionContext()->delay(::make_runnable([dispatcher]() {
                    dispatcher->stop();
                }), 0);
            });
        }));

        WAIT_AND_TIMEOUT(dispatcher, 10000)
    }
}

TEST(HttpClient, GETWithSax) {
    struct Success {
        int the_answer;
    };
    struct Failure {
        int code;
        bool not_here;
    };

    struct Handler : public HttpJsonHandler<std::shared_ptr<Success>, Failure, Handler> {
        Handler() {};

        bool Bool(bool b) {
            return true;
        };

        bool RawNumber(const Ch *str, rapidjson::SizeType len, bool copy) {
            if (_currentKey == std::string("the_answer"))
                _result.getRight()->the_answer = boost::lexical_cast<int>(std::string(str, len));
            return true;
        }

        bool Key(const Ch *str, rapidjson::SizeType len, bool copy) {
            _currentKey = std::string(str, len);
            if (getStatusCode() >= 200 && getStatusCode() < 300) {
                _result = std::make_shared<Success>();
            } else {
                _result = Failure();
            }
            return true;
        }

        Either<Failure, std::shared_ptr<Success>> build() {
            return _result;
        }

    private:
        std::string _currentKey;
        Either<Failure, std::shared_ptr<Success>> _result;

    };

    auto dispatcher = std::make_shared<NativeThreadDispatcher>();
    auto client = std::make_shared<MongooseHttpClient>(dispatcher->getSerialExecutionContext("client"));
    auto worker = dispatcher->getSerialExecutionContext("worker");
    ledger::core::HttpClient http("http://127.0.0.1:8000", client, worker);
    {
        auto server = std::make_shared<MongooseSimpleRestServer>(dispatcher->getSerialExecutionContext("server"));

        server->GET("/talk/to/me/in/json", [dispatcher](const RestRequest &request) -> RestResponse {
            return {200, "OK", "{\"the_answer\": 42}"};
        });

        server->start(8000);

        worker->execute(::make_runnable([&http, dispatcher, &server] () {
            http
                    .GET("/talk/to/me/in/json")
                    .json<Success, Failure>(Handler())
                    .foreach(dispatcher->getMainExecutionContext(), [&server, dispatcher] (const Either<Failure, std::shared_ptr<Success>>& result) {
                        EXPECT_TRUE(result.isRight());
                        EXPECT_EQ(result.getRight()->the_answer, 42);
                        server->stop();
                        dispatcher->getMainExecutionContext()->delay(::make_runnable([dispatcher]() {
                            dispatcher->stop();
                        }), 0);
                    });
        }));
        WAIT_AND_TIMEOUT(dispatcher, 10000);
    }
}

TEST(HttpClient, GETWithSaxError) {
    struct Success {
        int the_answer;
    };
    struct Failure {
        int code;
        bool not_here;
    };

    struct Handler : public HttpJsonHandler<std::shared_ptr<Success>, Failure, Handler> {
        Handler() {};

        bool Bool(bool b) {
            if (_currentKey == std::string("not_here"))
                _result.getLeft().not_here = b;
            return true;
        };

        bool Uint(unsigned int i) {
            if (_currentKey == std::string("the_answer"))
                _result.getRight()->the_answer = i;
            return true;
        }

        bool Int(int i) {
            if (_currentKey == std::string("the_answer"))
                _result.getRight()->the_answer = i;
            return true;
        }

        bool Key(const Ch *str, rapidjson::SizeType len, bool copy) {
            _currentKey = std::string(str, len);
            if (getStatusCode() >= 200 && getStatusCode() < 300) {
                _result = std::make_shared<Success>();

            } else {
                _result = Failure();
            }
            return true;
        }

        Either<Failure, std::shared_ptr<Success>> build() {
            return _result;
        }

    private:
        std::string _currentKey;
        Either<Failure, std::shared_ptr<Success>> _result;
    };

    auto dispatcher = std::make_shared<NativeThreadDispatcher>();
    auto client = std::make_shared<MongooseHttpClient>(dispatcher->getSerialExecutionContext("client"));
    auto worker = dispatcher->getSerialExecutionContext("worker");
    ledger::core::HttpClient http("http://127.0.0.1:8000", client, worker);
    {
        auto server = std::make_shared<MongooseSimpleRestServer>(dispatcher->getSerialExecutionContext("server"));

        server->GET("/knock/knock/neo", [dispatcher](const RestRequest &request) -> RestResponse {
            return {301, "Moved Permanently", "{\"not_here\": true}"};
        });

        server->start(8000);

        worker->execute(::make_runnable([&http, dispatcher, &server] () {
            http
                    .GET("/knock/knock/neo")
                    .json<Success, Failure>(Handler())
                    .foreach(dispatcher->getMainExecutionContext(), [&server, dispatcher] (const Either<Failure, std::shared_ptr<Success>>& result) {
                        EXPECT_TRUE(result.isLeft());
                        EXPECT_EQ(result.getLeft().not_here, true);
                        server->stop();
                        dispatcher->getMainExecutionContext()->delay(::make_runnable([dispatcher]() {
                            dispatcher->stop();
                        }), 0);
                    });
        }));
        WAIT_AND_TIMEOUT(dispatcher, 10000);
    }
}
