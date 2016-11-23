/*
 *
 * json_preferences_test
 * ledger-core
 *
 * Created by Pierre Pollastri on 21/11/2016.
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

static void sayHello(EventThread& worker) {
    worker.getLooper()->push_back(new LambdaEvent([&worker] () {
        std::cout << "Hello from worker" << std::endl;
        EXPECT_EQ("TOTO", "TOTO");
        sayHello(worker);
    }), 1000);
}

TEST(JsonPreferences, StoreAndGetPreferencesRaw) {
    auto resolver = std::make_shared<NativePathResolver>();
    // FIRST PASS: We are creating new preferences
    {
        auto dispatcher = std::make_shared<NativeThreadDispatcher>();
        auto preferencesLock = dispatcher->newLock();
        auto backend = std::make_shared<ledger::core::AtomicPreferencesBackend>(
                "/preferences/tests.json",
                dispatcher->getSerialExecutionContext("worker"),
                resolver,
                preferencesLock
        );

        dispatcher->getSerialExecutionContext("toto")->execute(make_runnable([=] () {
            backend->load([backend, dispatcher] () {
                std::vector<ledger::core::PreferencesChanges *> changes;
                {
                    auto change = new ledger::core::PreferencesChanges();
                    change->value.SetInt(42);
                    change->name = "my_number";
                    change->type = ledger::core::PreferencesChangeType::SET;
                    changes.push_back(change);
                }
                {
                    auto change = new ledger::core::PreferencesChanges();
                    change->value.SetString("I'm a wonderful string");
                    change->name = "my_string";
                    change->type = ledger::core::PreferencesChangeType::SET;
                    changes.push_back(change);
                }
                {
                    auto change = new ledger::core::PreferencesChanges();
                    change->value.SetString("I'm a string to remove");
                    change->name = "my_string_to_remove";
                    change->type = ledger::core::PreferencesChangeType::SET;
                    changes.push_back(change);
                }
                {
                    auto change = new ledger::core::PreferencesChanges();
                    change->name = "my_string_to_remove";
                    change->type = ledger::core::PreferencesChangeType::REMOVE;
                    changes.push_back(change);
                }
                backend->save("toto_is_great", changes);
                dispatcher->getSerialExecutionContext("worker")->execute(make_runnable([dispatcher] () {
                    dispatcher->stop();
                }));
            });
        }));

        dispatcher->getSerialExecutionContext("toto")->delay(make_runnable([dispatcher]() {
            dispatcher->stop();
            FAIL() << "Timeout";
        }), 5000);

        dispatcher->waitUntilStopped();
    }
    // SECOND PASS: We fetch previously created preferences
    {
        auto dispatcher = std::make_shared<NativeThreadDispatcher>();
        auto preferencesLock = dispatcher->newLock();
        auto backend = std::make_shared<ledger::core::AtomicPreferencesBackend>(
                "/preferences/tests.json",
                dispatcher->getSerialExecutionContext("worker"),
                resolver,
                preferencesLock
        );

        dispatcher->getSerialExecutionContext("toto")->execute(make_runnable([=] () {
            backend->load([backend, dispatcher] () {
                {
                    auto number = backend->getObject("toto_is_great")->FindMember("my_number");
                    EXPECT_TRUE(number->value.IsNumber());
                    EXPECT_EQ(number->value.GetInt(), 42);
                }
                {
                    auto string = backend->getObject("toto_is_great")->FindMember("my_string");
                    EXPECT_TRUE(string->value.IsString());
                    EXPECT_STREQ(string->value.GetString(), "I'm a wonderful string");
                }
                    dispatcher->getSerialExecutionContext("worker")->execute(make_runnable([dispatcher] () {
                    dispatcher->stop();
                }));
            });
        }));

        dispatcher->getSerialExecutionContext("toto")->delay(make_runnable([dispatcher]() {
            dispatcher->stop();
            FAIL() << "Timeout";
        }), 5000);

        dispatcher->waitUntilStopped();
    }
    resolver->clean();
}