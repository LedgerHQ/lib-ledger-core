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
#include <ledger/core/preferences/PreferencesBackend.hpp>
#include <NativePathResolver.hpp>
#include <fstream>

TEST(JsonPreferences, StoreAndGetWithPreferencesAPI) {
    auto resolver = std::make_shared<NativePathResolver>();
    auto dispatcher = std::make_shared<NativeThreadDispatcher>();
    auto preferencesLock = dispatcher->newLock();
    auto backend = std::make_shared<ledger::core::PreferencesBackend>(
            "/preferences/tests.db",
            dispatcher->getSerialExecutionContext("worker"),
            resolver
    );
    auto preferences = backend->getPreferences("my_test_preferences");
    dispatcher->getSerialExecutionContext("not_my_worker")->execute(make_runnable([=] () {
        preferences->edit()
                ->putString("my_string", "Hello World!")
                ->putBoolean("my_bool", true)
                ->putInt("my_int", 42)
                ->putLong("my_long", 42LL << 42LL)
                ->putStringArray("my_string_array", std::vector<std::string>({"Hello", "world", "!"}))
                ->commit();
    }));
    dispatcher->getSerialExecutionContext("worker")->execute(make_runnable([=] () {
        preferences
                ->edit()
                ->putString("my_worker_string", "Hey World!")
                ->putString("my_string_to_remove", "Remove this please!")
                ->commit();
    }));
    // Assume that 100ms should be enough to persist data
    dispatcher->getMainExecutionContext()->delay(make_runnable([=] () {
        EXPECT_EQ(preferences->getString("my_string", ""), "Hello World!");
        EXPECT_EQ(preferences->getString("my_worker_string", ""), "Hey World!");
        EXPECT_EQ(preferences->getString("my_fake_string", "Not My String"), "Not My String");
        EXPECT_EQ(preferences->getString("my_string_to_remove", ""), "Remove this please!");
        EXPECT_EQ(preferences->getInt("my_int", -1), 42);
        EXPECT_EQ(preferences->getLong("my_long", -1),  42LL << 42LL);
        EXPECT_EQ(preferences->getBoolean("my_bool", false), true);
        EXPECT_EQ(preferences->getStringArray("my_string_array", {}), std::vector<std::string>({"Hello", "world", "!"}));
        preferences->edit()->remove("my_string_to_remove")->commit();
        dispatcher->getSerialExecutionContext("worker")->delay(make_runnable([=] () {
            EXPECT_EQ(preferences->getString("my_string_to_remove", "Removed"), "Removed");
            dispatcher->stop();
        }), 50);
    }), 100);
    dispatcher->getSerialExecutionContext("toto")->delay(make_runnable([dispatcher]() {
        dispatcher->stop();
        FAIL() << "Timeout";
    }), 5000);

    dispatcher->waitUntilStopped();
    resolver->clean();
}
